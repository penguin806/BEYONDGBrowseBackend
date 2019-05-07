#ifndef PROTEINQUERY_H
#define PROTEINQUERY_H

#include <QString>
#include <QtSql>

class ProteinQuery
{
public:
    ProteinQuery();
    bool connectToDataBase();
    QString queryProteinUniprotId(QString name, QString posStart, QString posEnd);

private:
    QString serverAddrress;
    QString databaseName;
    QString username;
    QString password;

    QSqlDatabase proteinDB;
};

#endif // PROTEINQUERY_H
