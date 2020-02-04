#ifndef HTTPSERVICE_H
#define HTTPSERVICE_H

#include <QtHttpServer>
#include "databasequery.h"

class HttpService
{
public:
    HttpService(QSqlDatabase databaseConnection);
    bool startListening();

protected:
    void loadHttpServiceConfig();
    void initUrlRouting();
    void writeResponseData(
                QHttpServerResponder &responder,
                const QJsonDocument &document,
                QHttpServerResponder::StatusCode status = QHttpServerResponder::StatusCode::Ok
            );
    QJsonArray queryProteinByReferenceSequenceRegion(quint16 datasetId, QString proteinName, QString position);
    QJsonArray queryRegionByProteinId(quint16 datasetId, QString proteinName);
    QJsonArray queryAnnotationBySequenceRegion(quint16 datasetId, QString name, QString posStart, QString posEnd);
    bool insertSequenceAnnotationAtSpecificPosition(quint16 datasetId, qint32 id, QString name, qint32 position, QString time, QString contents, QString authorUsername, QString remoteAddress);
    QJsonArray queryDatasetsList();
    QJsonArray queryProteinIdListForAutoComplete(quint16 datasetId, QString proteinName);
    QJsonArray searchAnnotation(quint16 datasetId, qint32 id, QString contents, QString authorUsername, QString remoteAddress);

private:
    QHttpServer snowHttpServer;
    quint16 listenPort = 12080;
    DatabaseQuery databaseQuery;
};

#endif // HTTPSERVICE_H
