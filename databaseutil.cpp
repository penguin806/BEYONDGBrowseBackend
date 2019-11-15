#include "databaseutil.h"
#include <QSettings>
#include <QSqlDatabase>
#include <QTextCodec>
#include <QDebug>

DataBaseUtil::DataBaseUtil()
{
    this->loadDatabaseConfig();
}

bool DataBaseUtil::connectToDataBase()
{
    bool isMysqlAvailable = QSqlDatabase::isDriverAvailable("QMYSQL");
    if(!isMysqlAvailable)
    {
        qDebug() << "[Info] Available database driver: " << QSqlDatabase::drivers();
        qDebug() << "[Error] QMYSQL Driver Not Available!";
        return false;
    }
    QSqlDatabase databaseConnection =
            QSqlDatabase::addDatabase("QMYSQL", "beyondgbrowse");
    databaseConnection.setHostName(this->serverAddrress);
    databaseConnection.setDatabaseName(this->databaseName);
    databaseConnection.setUserName(this->username);
    databaseConnection.setPassword(this->password);

    bool isOpenDatabaseSuccess = databaseConnection.open();
    if(isOpenDatabaseSuccess)
    {
        qDebug() << "[Info] Connect to database success!";
        this->databaseConnection = databaseConnection;
    }
    else
    {
        qDebug() << "[Error] Connect to database fail!";
    }

    this->printDatabaseConfig();
    return isOpenDatabaseSuccess;
}

QSqlDatabase DataBaseUtil::getDatabaseConnection()
{
    return this->databaseConnection;
}

void DataBaseUtil::loadDatabaseConfig()
{
    QSettings snowDbSettings("./config.ini", QSettings::IniFormat);
    snowDbSettings.setIniCodec(QTextCodec::codecForName("UTF-8"));
    snowDbSettings.beginGroup("Database");

    this->serverAddrress = snowDbSettings.value(
        QString("serverAddrress"),
        QString("localhost")
    ).toString();

    this->databaseName = snowDbSettings.value(
        QString("databaseName"),
        QString("beyondgbrowse")
    ).toString();

    this->username = snowDbSettings.value(
        QString("username"),
        QString("snow_db201905")
    ).toString();

    this->password = snowDbSettings.value(
        QString("password"),
        QString("snow_db201905")
    ).toString();

    snowDbSettings.endGroup();
}

void DataBaseUtil::printDatabaseConfig()
{
    qDebug() << "DatabaseUrl: " << this->serverAddrress;
    qDebug() << "DatabaseName: " << this->databaseName;
    qDebug() << "Username: " << this->username;
    qDebug() << "Password: " << this->password;
}
