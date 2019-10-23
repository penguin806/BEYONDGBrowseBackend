#include <QtHttpServer/QHttpServer>
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

    this->listenPort = snowHttpSettings.value(
        QString("listenPort"),
        12080
    ).toInt();

    snowHttpSettings.endGroup();
}

void HttpService::initUrlRouting()
{
    this->snowHttpServer.route("/", [](){
        return "Welcome to BeyondGBrowse web interface!";
    });

    // http://localhost:12080/ref/chr1/149813549..149813576
    // 取回位置区间对应的蛋白质变体信息
    // 输入： 参考序列名称、起始位置、终止位置
    // 返回Json数组，每个对象包含：质荷比数组arrMSScanMassArray、峰度数组arrMSScanPeakAundance、起始位置_start、终止位置end、蛋白质变体序列sequence、正反链strand、scanId、uniprot_id
    //
    this->snowHttpServer.route("/ref/<arg>/<arg>", [this](QString proteinName, QString position){
        QJsonArray recordArray;
        try {
            recordArray = this->queryProteinByReferenceSequenceRegion(proteinName, position);

        } catch (QString errorReason) {
            errorReason = "[Warning] " + QString("/ref/%1/%2 :")
                    .arg(proteinName, position) + errorReason;
            qDebug() << errorReason.toUtf8().data();
        }

        return recordArray;
    });

    // http://localhost:12080/locate/H32_HUMAN
    // 查找蛋白质对应的起始&终止位置
    // 输入： EnsembleId或UniprotId、起始位置、终止位置
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

    this->snowHttpServer.route("/locate/<arg>", [this](QString proteinName){
        QJsonArray recordArray;
        try {
            recordArray = this->queryRegionByProteinId(proteinName);

        } catch (QString errorReason) {
            errorReason = "[Warning] " + QString("/locate/%1 :")
                    .arg(proteinName) + errorReason;
            qDebug() << errorReason.toUtf8().data();
        }

        return recordArray;
    });

    // http://localhost:12080/annotation/query/Scan998/85..92
    // 获取特定范围内所有注释
    // 输入： 参考序列名称/蛋白质变体scanId、起始位置、终止位置
    // 返回Json数组，Eg:
    // [
    //    {
    //       "contents":"TEST",
    //       "name":"Scan998",
    //       "position":"89",
    //       "time":"2019-10-13T16:24:01.000"
    //    }
    // ]
    this->snowHttpServer.route("/annotation/query/<arg>/<arg>", [this](QString name, QString position){
        QJsonArray recordArray;
        try {
            QStringList posList = position.split("..");
            recordArray = this->queryAnnotationBySequenceRegion(name, posList.at(0), posList.at(1));

        } catch (QString errorReason) {
            errorReason = "[Warning] " + QString("/annotation/query/%1/%2 :")
                    .arg(name, position) + errorReason;
            qDebug() << errorReason.toUtf8().data();
        }

        return recordArray;
    });

    // http://localhost:12080/annotation/insert/Scan1053/65/2019-10-13 16:39:26/TEST
    // 在特定位置插入注释
    // 输入： 参考序列名称/蛋白质变体scanId、位置、时间、内容
    // 成功返回SUCCESS、失败返回FAIL
    //
    this->snowHttpServer.route("/annotation/insert/<arg>/<arg>/<arg>/<arg>", [this](QString name, qint32 position, QString time, QString contents){
        bool isInsertSuccess;
        try {
            isInsertSuccess = this->insertSequenceAnnotationAtSpecificPosition(
                        0, name, position, time, contents);

        } catch (QString errorReason) {
            errorReason = "[Warning] " + QString("/annotation/insert/%1/%2/%3/... :")
                    .arg(name, QString::number(position), time) + errorReason;
            qDebug() << errorReason.toUtf8().data();
        }

        return isInsertSuccess ? QString("SUCCESS") : QString("FAIL");
    });
}

QJsonArray HttpService::queryProteinByReferenceSequenceRegion(
            QString proteinName, QString position
        )
{
    QStringList posList = position.split("..");
    if(posList.size() != 2)
    {
        throw QString("ERROR_QUERY_ARGUMENT_INVALID");
    }
    QJsonArray result = this->databaseQuery
            .queryProteinBySequenceRegion(
                proteinName,posList.at(0),posList.at(1)
             );

    if(result.isEmpty())
    {
        throw QString("NOT_FOUND");
    }
    return result;
}

QJsonArray HttpService::queryRegionByProteinId(QString proteinName)
{
    if(proteinName.isEmpty())
    {
        throw QString("ERROR_QUERY_ARGUMENT_INVALID");
    }
    QJsonArray result = this->databaseQuery
            .queryRegionByProteinId(proteinName);

    if(result.isEmpty())
    {
        throw QString("NOT_FOUND");
    }
    return result;
}

QJsonArray HttpService::queryAnnotationBySequenceRegion(QString name, QString posStart, QString posEnd)
{
    if(name.isEmpty() || posStart.isEmpty() || posEnd.isEmpty())
    {
        throw QString("ERROR_QUERY_ARGUMENT_INVALID");
    }
    QJsonArray result = this->databaseQuery
            .queryAnnotationBySequenceRegion(name, posStart, posEnd);

    if(result.isEmpty())
    {
        throw QString("NOT_FOUND");
    }
    return result;
}

bool HttpService::insertSequenceAnnotationAtSpecificPosition(qint32 id, QString name, qint32 position, QString time, QString contents)
{
    if(name.isEmpty() || time.isEmpty() || contents.isEmpty())
    {
        throw QString("ERROR_QUERY_ARGUMENT_INVALID");
    }
    bool result = this->databaseQuery
            .insertSequenceAnnotationAtSpecificPosition(
                0, name, position, time, contents
            );

    return result;
}
