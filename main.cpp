#include <QCoreApplication>
#include <QtHttpServer/QHttpServer>
#include "proteinquery.h"

ProteinQuery query;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("SnowPluginBackend");
    QCoreApplication::setApplicationVersion("0.1.0");
    QCoreApplication::setOrganizationDomain("xuefeng.space");

    qDebug() << "Connecting to database...";
    if( !query.connectToDataBase() )
    {
        qDebug() << "Connect to database FAIL!";
        return -1;
    }

    QHttpServer snowHttpServer;
    snowHttpServer.route("/", [](){
        return "SnowTest20190507!";
    });

    snowHttpServer.route("/ref/<arg>/<arg>", [](QString proteinName, QString position){
        QStringList posList = position.split("..");
        if(posList.size() != 2)
        {
            return QString("ERROR_QUERY_ARGUMENT_INVALID");
        }
        QString result = query.queryProteinByReferenceSequenceRegion(
                    proteinName,posList.at(0),posList.at(1));
        qDebug() << "Database query result:" << result.left(10);

        if(result.isEmpty())
        {
            return QString("ERROR_NOT_FOUND");
        }
        return result;
    });

    snowHttpServer.route("/locate/<arg>", [](QString proteinName){
        if(proteinName.isEmpty())
        {
            return QString("ERROR_QUERY_ARGUMENT_INVALID");
        }
        QString result = query.queryRegionByProteinId(proteinName);
        qDebug() << "Database query result:" << result.left(10);

        if(result.isEmpty())
        {
            return QString("ERROR_NOT_FOUND");
        }
        return result;
    });

    const int listenPort =
            snowHttpServer.listen(QHostAddress::Any,12080);

    if(listenPort == -1)
    {
        qDebug() << QString("Could not run on http://0.0.0.0:%1/").arg(listenPort);
                return 0;
    }
    else {
        qDebug() << QString("Running on http://0.0.0.0:%1/ (Press CTRL+C to quit)").arg(listenPort);
    }


    return a.exec();
}
