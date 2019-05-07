#include <QCoreApplication>
#include <QtHttpServer/QHttpServer>
#include "proteinquery.h"

ProteinQuery query;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

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

    snowHttpServer.route("/<arg>/<arg>/uniprot_id", [](QString proteinName, QString position){
        QStringList posList = position.split("..");
        if(posList.size() != 2)
        {
            return QString();
        }
        QString result = query.queryProteinUniprotId(
                    proteinName,posList.at(0),posList.at(1));
        qDebug() << "Database query result:" << result;

        return result;
    });

    const int listenPort =
            snowHttpServer.listen(QHostAddress::Any,12345);

    if(listenPort == -1)
    {
        qDebug() << QString("Could not run on http://0.0.0.1:%1/").arg(listenPort);
                return 0;
    }
    else {
        qDebug() << QString("Running on http://0.0.0.1:%1/ (Press CTRL+C to quit)").arg(listenPort);
    }


    return a.exec();
}
