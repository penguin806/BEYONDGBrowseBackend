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

    QHttpServer snowHttpServer;
    const int listenPort = 12080;
    DatabaseQuery databaseQuery;
};

#endif // HTTPSERVICE_H
