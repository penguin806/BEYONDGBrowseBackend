#ifndef PROTEINQUERY_H
#define PROTEINQUERY_H

#include <QSqlDatabase>
#include <QJsonArray>

class DatabaseQuery
{
public:
    DatabaseQuery(QSqlDatabase databaseConnection);
    QJsonArray queryProteinBySequenceRegion(quint16 datasetId, QString name, QString posStart, QString posEnd);
    QJsonArray queryRegionByProteinId(quint16 datasetId, QString proteinName);
    QJsonArray queryAnnotationBySequenceRegion(quint16 datasetId, QString name, QString posStart, QString posEnd);
    bool insertSequenceAnnotationAtSpecificPosition(quint16 datasetId, qint32 id, QString name, qint32 position, QString time, QString contents, QString authorUsername, QString remoteAddress);
    QJsonArray queryDatasetsList();
    QJsonArray queryProteinIdListForAutoComplete(quint16 datasetId, QString proteinName);
    QJsonArray searchAnnotation(quint16 datasetId, qint32 id, QString contents, QString authorUsername, QString remoteAddress);

private:
    QSqlDatabase databaseConnection;
};

#endif // PROTEINQUERY_H
