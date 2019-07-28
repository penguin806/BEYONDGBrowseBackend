#ifndef PROTEINQUERY_H
#define PROTEINQUERY_H

#include <QString>
#include <QSqlDatabase>
#include <QJsonArray>

class DatabaseQuery
{
public:
    DatabaseQuery(QSqlDatabase databaseConnection);
    QJsonArray queryProteinByReferenceSequenceRegion(QString name, QString posStart, QString posEnd);
    QJsonArray queryRegionByProteinId(QString proteinName);

private:
    QSqlDatabase databaseConnection;
};

#endif // PROTEINQUERY_H
