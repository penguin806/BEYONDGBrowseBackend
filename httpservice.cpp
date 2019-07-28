#include <QtHttpServer/QHttpServer>
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
            return recordArray;

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
            .queryProteinByReferenceSequenceRegion(
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
