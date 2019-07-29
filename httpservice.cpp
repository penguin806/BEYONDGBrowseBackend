#include <QtHttpServer/QHttpServer>
#include <QJsonDocument>
#include "httpservice.h"
#include "databasequery.h"

HttpService::HttpService(QSqlDatabase databaseConnection) :
    databaseQuery(DatabaseQuery(databaseConnection))
{
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

void HttpService::initUrlRouting()
{
    this->snowHttpServer.route("/", [](){
        return "Welcome to BeyondGBrowse web interface!";
    });

    this->snowHttpServer.route("/ref/<arg>/<arg>", [this](QString proteinName, QString position){
        QJsonArray recordArray;
        try {
            recordArray = this->queryProteinByReferenceSequenceRegion(proteinName, position);

        } catch (QString errorReason) {
            errorReason = "[Error] " + QString("/ref/%1/%2 :")
                    .arg(proteinName, position) + errorReason;
            qDebug() << errorReason.toUtf8().data();
        }

        return recordArray;
    });

    this->snowHttpServer.route("/locate/<arg>", [this](QString proteinName){
        QJsonArray recordArray;
        try {
            recordArray = this->queryRegionByProteinId(proteinName);

        } catch (QString errorReason) {
            errorReason = "[Error] " + QString("/locate/%1 :")
                    .arg(proteinName) + errorReason;
            qDebug() << errorReason.toUtf8().data();
        }

        return recordArray;
    });

    this->snowHttpServer.route("/annotation/query/<arg>/<arg>", [this](QString name, QString position){
        QJsonArray recordArray;
        try {
            QStringList posList = position.split("..");
            recordArray = this->queryAnnotationBySequenceRegion(name, posList.at(0), posList.at(1));

        } catch (QString errorReason) {
            errorReason = "[Error] " + QString("/annotation/query/%1/%2 :")
                    .arg(name, position) + errorReason;
            qDebug() << errorReason.toUtf8().data();
        }

        return recordArray;
    });

    this->snowHttpServer.route("/annotation/insert/<arg>/<arg>/<arg>/<arg>", [this](QString name, qint32 position, QString time, QString contents){
        bool isInsertSuccess;
        try {
            isInsertSuccess = this->insertSequenceAnnotationAtSpecificPosition(
                        0, name, position, time, contents);

        } catch (QString errorReason) {
            errorReason = "[Error] " + QString("/annotation/insert/%1/%2/%3/... :")
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
        throw QString("ERROR_NOT_FOUND");
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
        throw QString("ERROR_NOT_FOUND");
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
        throw QString("ERROR_NOT_FOUND");
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
