#ifndef DATABASEUTIL_H
#define DATABASEUTIL_H
#include <QString>
#include <QSqlDatabase>

class DataBaseUtil
{
public:
    DataBaseUtil();
    bool connectToDataBase();
    QSqlDatabase getDatabaseConnection();
    void loadDatabaseConfig();
    void printDatabaseConfig();

private:
    QString serverAddrress;
    QString databaseName;
    QString username;
    QString password;

    QSqlDatabase databaseConnection;
};

#endif // DATABASEUTIL_H
