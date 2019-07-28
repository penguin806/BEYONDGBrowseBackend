#include <QCoreApplication>
#include "databaseutil.h"
#include "httpservice.h"

int main(int argc, char *argv[])
{
    QCoreApplication application(argc, argv);
    QCoreApplication::setApplicationName("SnowPluginBackend");
    QCoreApplication::setApplicationVersion("0.1.0");
    QCoreApplication::setOrganizationDomain("xuefeng.space");

    DataBaseUtil dbUtil;
    bool bSuccess = dbUtil.connectToDataBase();
    if(!bSuccess)
    {
        return 0x01;
    }

    HttpService httpService(dbUtil.getDatabaseConnection());
    bSuccess = httpService.startListening();
    if(!bSuccess)
    {
        return 0x02;
    }

    return application.exec();
}
