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
    QJsonArray queryProteinByReferenceSequenceRegion(quint16 datasetId, QString proteinName, QString position);
    QJsonArray queryRegionByProteinId(quint16 datasetId, QString proteinName);
    QJsonArray queryAnnotationBySequenceRegion(quint16 datasetId, QString name, QString posStart, QString posEnd);
    bool insertSequenceAnnotationAtSpecificPosition(quint16 datasetId, qint32 id, QString name, qint32 position, QString time, QString contents, QString authorUsername, QString remoteAddress);
    QJsonArray queryDatasetsList();

private:
    QHttpServer snowHttpServer;
    quint16 listenPort = 12080;
    DatabaseQuery databaseQuery;
};

#endif // HTTPSERVICE_H
