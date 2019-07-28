#ifndef PROTEINQUERY_H
#define PROTEINQUERY_H

#include <QSqlDatabase>
#include <QJsonArray>

class DatabaseQuery
{
public:
    DatabaseQuery(QSqlDatabase databaseConnection);
    QJsonArray queryProteinBySequenceRegion(QString name, QString posStart, QString posEnd);
    QJsonArray queryRegionByProteinId(QString proteinName);
    QJsonArray queryAnnotationBySequenceRegion(QString name, QString posStart, QString posEnd);
    bool insertSequenceAnnotationAtSpecificPosition(qint32 id, QString name, qint32 position, QString time, QJsonArray contents);

private:
    QSqlDatabase databaseConnection;
};

#endif // PROTEINQUERY_H
