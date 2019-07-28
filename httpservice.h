#ifndef HTTPSERVICE_H
#define HTTPSERVICE_H

#include <QtHttpServer/QHttpServer>
#include "databasequery.h"

class HttpService
{
public:
    HttpService(QSqlDatabase databaseConnection);
    bool startListening();

private:
    void initUrlRouting();
    QJsonArray queryProteinByReferenceSequenceRegion(QString proteinName, QString position);
    QJsonArray queryRegionByProteinId(QString proteinName);
    QJsonArray queryAnnotationBySequenceRegion(QString name, QString posStart, QString posEnd);
    bool insertSequenceAnnotationAtSpecificPosition(qint32 id, QString name, qint32 position, QString time, QJsonArray contents);


    QHttpServer snowHttpServer;
    const int listenPort = 12080;
    DatabaseQuery databaseQuery;
};

#endif // HTTPSERVICE_H
