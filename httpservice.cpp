#include <QtHttpServer>
#include <QJsonDocument>
#include <QSettings>
#include <QTextCodec>
#include "httpservice.h"
#include "databasequery.h"

HttpService::HttpService(QSqlDatabase databaseConnection) :
    databaseQuery(DatabaseQuery(databaseConnection))
{
    this->loadHttpServiceConfig();
    this->initUrlRouting();
}

bool HttpService::startListening()
{
    int portResult =
            snowHttpServer.listen(QHostAddress::Any, this->listenPort);

    if(portResult != this->listenPort)
    {
        QString errorMsg = QString("[Error] Could not run on http://0.0.0.0:%1 Code:%2")
                .arg(this->listenPort, portResult);
        qDebug() << errorMsg.toUtf8().data();
        return false;
    }
    else
    {
        QString successMsg = QString("[Info] Running on http://0.0.0.0:%1/ Success!")
                .arg(this->listenPort);
        qDebug() << successMsg.toUtf8().data();
        return true;
    }
}

void HttpService::loadHttpServiceConfig()
{
    QSettings snowHttpSettings("./config.ini", QSettings::IniFormat);
    snowHttpSettings.setIniCodec(QTextCodec::codecForName("UTF-8"));
    snowHttpSettings.beginGroup("General");

    uint listenPort = snowHttpSettings.value(
        QString("listenPort"),
        12080
    ).toUInt();
    this->listenPort = quint16(listenPort);

    snowHttpSettings.endGroup();
}

// Encapsulate the process of writing data,
// Add request headers to resolve 'CORS (Cross-Origin Resource Sharing)' issue on modern browsers
void HttpService::writeResponseData(QHttpServerResponder &responder, const QJsonDocument &document, QHttpServerResponder::StatusCode status)
{
    std::initializer_list<std::pair<QByteArray, QByteArray>> headerList =
    {
        std::pair<QByteArray, QByteArray>(
            QByteArrayLiteral("Access-Control-Allow-Origin"),
            QByteArrayLiteral("*")
        ),
        std::pair<QByteArray, QByteArray>(
            QByteArrayLiteral("Access-Control-Allow-Methods"),
            QByteArrayLiteral("GET, POST")
        )
    };
    responder.write(document, headerList);
}

void HttpService::initUrlRouting()
{
    this->snowHttpServer.route("/", [](){
        return "Welcome to BeyondGBrowse web interface!";
    });

    // http://localhost:12080/ref/1/chr1/149813549..149813576
    // 取回位置区间对应的蛋白质变体信息
    // 输入： 数据集ID、参考序列名称、起始位置、终止位置
    // 返回Json数组，每个对象包含：质荷比数组arrMSScanMassArray、峰度数组arrMSScanPeakAundance、起始位置_start、终止位置end、蛋白质变体序列sequence、正反链strand、scanId、uniprot_id
    //
    this->snowHttpServer.route("/<arg>/ref/<arg>/<arg>", [this](quint16 datasetId, QString proteinName, QString position, QHttpServerResponder &&responder){
        QJsonArray recordArray;
        try {
            recordArray = this->queryProteinByReferenceSequenceRegion(datasetId, proteinName, position);

        } catch (QString errorReason) {
            errorReason = "[Warning] " + QString("/ref/%1/%2 :")
                    .arg(proteinName, position) + errorReason;
            qDebug() << errorReason.toUtf8().data();
        }

        this->writeResponseData(responder, QJsonDocument(recordArray));
    });

    // http://localhost:12080/1/locate/H32_HUMAN
    // 查找蛋白质对应的起始&终止位置
    // 输入： 数据集ID、EnsembleId或UniprotId、起始位置、终止位置
    // 返回Json数组，Eg:
    // [
    //    {
    //       "_start":"149813271",
    //       "end":"149813681",
    //       "name":"chr1"
    //    },
    //    {
    //       "_start":"149839538",
    //       "end":"149841193",
    //       "name":"chr1"
    //    },
    //    {
    //       "_start":"149852619",
    //       "end":"149854274",
    //       "name":"chr1"
    //    }
    // ]

    this->snowHttpServer.route("/<arg>/locate/<arg>", [this](quint16 datasetId, QString proteinName, QHttpServerResponder &&responder){
        QJsonArray recordArray;
        try {
            recordArray = this->queryRegionByProteinId(datasetId, proteinName);

        } catch (QString errorReason) {
            errorReason = "[Warning] " + QString("/locate/%1 :")
                    .arg(proteinName) + errorReason;
            qDebug() << errorReason.toUtf8().data();
        }

        this->writeResponseData(responder, QJsonDocument(recordArray));
    });

    // http://localhost:12080/1/annotation/query/Scan998/85..92
    // 获取特定范围内所有注释
    // 输入： 数据集ID、参考序列名称/蛋白质变体scanId、起始位置、终止位置
    // 返回Json数组，Eg:
    // [
    //    {
    //       "contents":"TEST",
    //       "name":"Scan998",
    //       "position":"89",
    //       "time":"2019-10-13T16:24:01.000"
    //    }
    // ]
    this->snowHttpServer.route("/<arg>/annotation/query/<arg>/<arg>", [this](quint16 datasetId, QString name, QString position, QHttpServerResponder &&responder){
        QJsonArray recordArray;
        try {
            QStringList posList = position.split("..");
            recordArray = this->queryAnnotationBySequenceRegion(datasetId, name, posList.at(0), posList.at(1));

        } catch (QString errorReason) {
            errorReason = "[Warning] " + QString("/annotation/query/%1/%2 :")
                    .arg(name, position) + errorReason;
            qDebug() << errorReason.toUtf8().data();
        }

        this->writeResponseData(responder, QJsonDocument(recordArray));
    });

    // http://localhost:12080/annotation/insert/Scan1053/65/2019-10-13 16:39:26/TEST
    // 在特定位置插入注释
    // 输入： 参考序列名称/蛋白质变体scanId、位置、时间、内容
    // 成功返回SUCCESS、失败返回FAIL
    //
    this->snowHttpServer.route("/<arg>/annotation/insert/<arg>/<arg>/<arg>/<arg>", [this](quint16 datasetId, QString name, qint32 position, QString time, QString contents, const QHttpServerRequest &request, QHttpServerResponder &&responder){
//        QByteArray value = request.value("Host");
//        QUrl url = request.url();
//        QVariantMap headers = request.headers();
//        QByteArray body = request.body();
        QString authorUsername = request.query().queryItemValue("author");
        QString remoteAddress = request.remoteAddress().toString();

        bool isInsertSuccess;
        try {
            isInsertSuccess = this->insertSequenceAnnotationAtSpecificPosition(
                        datasetId, 0, name, position, time, contents, authorUsername, remoteAddress);

        } catch (QString errorReason) {
            errorReason = "[Warning] " + QString("/annotation/insert/%1/%2/%3/... :")
                    .arg(name, QString::number(position), time) + errorReason;
            qDebug() << errorReason.toUtf8().data();
        }

        if(isInsertSuccess)
        {
            QJsonObject jsonObjectToResponse({ QPair<QString, QJsonValue>(QString("status"), QString("SUCCESS")) });
            QJsonDocument jsonDocumentToResponse(jsonObjectToResponse);
            this->writeResponseData(responder, jsonDocumentToResponse);
        }
        else
        {
            QJsonObject jsonObjectToResponse({ QPair<QString, QJsonValue>(QString("status"), QString("FAIL")) });
            QJsonDocument jsonDocumentToResponse(jsonObjectToResponse);
            this->writeResponseData(responder, jsonDocumentToResponse);
        }
    });

    // http://localhost:12080/datasets
    // 查询数据集索引
    // 输入： 无
    // 返回Json数组： 当前数据库中所有数据集的id与对应name
    //
    this->snowHttpServer.route("/datasets", [this](QHttpServerResponder &&responder){
        QJsonArray recordArray;
        try {
            recordArray = this->queryDatasetsList();

        } catch (QString errorReason) {
            errorReason = "[Warning] " + QString("/datasets :")
                    + errorReason;
            qDebug() << errorReason.toUtf8().data();
        }

        this->writeResponseData(responder, QJsonDocument(recordArray));
    });

    // http://localhost:12080/1/locate_autocomplete/H32
    // 查找所有以xxx开头的蛋白质ID
    // 输入： 数据集ID、 UniprotId
    // 返回Json数组，Eg:
    // [
    //     "H32_HUMAN",
    //     "H0YH32_HUMAN",
    //     "KLH32_HUMAN",
    //     "A0A0G2JH32_HUMAN"
    // ]
    this->snowHttpServer.route("/<arg>/locate_autocomplete/<arg>", [this](quint16 datasetId, QString proteinName, QHttpServerResponder &&responder){
        QJsonArray recordArray;
        try {
            recordArray = this->queryProteinIdListForAutoComplete(datasetId, proteinName);

        } catch (QString errorReason) {
            errorReason = "[Warning] " + QString("/locate_autocomplete/%1 :")
                    .arg(proteinName) + errorReason;
            qDebug() << errorReason.toUtf8().data();
        }

        this->writeResponseData(responder, QJsonDocument(recordArray));
    });
}



QJsonArray HttpService::queryProteinByReferenceSequenceRegion(
            quint16 datasetId, QString proteinName, QString position
        )
{
    QStringList posList = position.split("..");
    if(posList.size() != 2)
    {
        throw QString("ERROR_QUERY_ARGUMENT_INVALID");
    }
    QJsonArray result = this->databaseQuery
            .queryProteinBySequenceRegion(
                datasetId, proteinName,posList.at(0),posList.at(1)
             );

    if(result.isEmpty())
    {
        throw QString("NOT_FOUND");
    }
    return result;
}

QJsonArray HttpService::queryRegionByProteinId(quint16 datasetId, QString proteinName)
{
    if(proteinName.isEmpty())
    {
        throw QString("ERROR_QUERY_ARGUMENT_INVALID");
    }
    QJsonArray result = this->databaseQuery
            .queryRegionByProteinId(datasetId, proteinName);

    if(result.isEmpty())
    {
        throw QString("NOT_FOUND");
    }
    return result;
}

QJsonArray HttpService::queryAnnotationBySequenceRegion(quint16 datasetId, QString name, QString posStart, QString posEnd)
{
    if(name.isEmpty() || posStart.isEmpty() || posEnd.isEmpty())
    {
        throw QString("ERROR_QUERY_ARGUMENT_INVALID");
    }
    QJsonArray result = this->databaseQuery
            .queryAnnotationBySequenceRegion(datasetId, name, posStart, posEnd);

    if(result.isEmpty())
    {
        throw QString("NOT_FOUND");
    }
    return result;
}

bool HttpService::insertSequenceAnnotationAtSpecificPosition(quint16 datasetId, qint32 id, QString name, qint32 position, QString time, QString contents, QString authorUsername, QString remoteAddress)
{
    if(name.isEmpty() || time.isEmpty() || contents.isEmpty())
    {
        throw QString("ERROR_QUERY_ARGUMENT_INVALID");
    }
    bool result = this->databaseQuery
            .insertSequenceAnnotationAtSpecificPosition(
                datasetId, 0, name, position, time, contents, authorUsername, remoteAddress
            );

    return result;
}

QJsonArray HttpService::queryDatasetsList()
{
    return this->databaseQuery.queryDatasetsList();
}

QJsonArray HttpService::queryProteinIdListForAutoComplete(quint16 datasetId, QString proteinName)
{
    if(proteinName.isEmpty())
    {
        throw QString("ERROR_QUERY_ARGUMENT_INVALID");
    }
    QJsonArray result = this->databaseQuery
            .queryProteinIdListForAutoComplete(datasetId, proteinName);

    if(result.isEmpty())
    {
        throw QString("NOT_FOUND");
    }
    return result;
}
