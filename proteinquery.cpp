#include "proteinquery.h"

#include <QtSql>

ProteinQuery::ProteinQuery()
{
    //this->serverAddrress = "server-hk1.xuefeng.space";
    this->serverAddrress = "172.25.176.243";
    this->databaseName = "snow_db201905";
    this->username = "snow_db201905";
    this->password = "201905";
}

bool ProteinQuery::connectToDataBase()
{
    this->proteinDB = QSqlDatabase::addDatabase("QMYSQL");
    this->proteinDB.setHostName(this->serverAddrress);
    this->proteinDB.setDatabaseName(this->databaseName);
    this->proteinDB.setUserName(this->username);
    this->proteinDB.setPassword(this->password);
    return this->proteinDB.open();
}

QString ProteinQuery::queryProteinUniprotId(QString name, QString posStart, QString posEnd)
{
    QSqlQuery query;
//    query.exec(
//                QString("SELECT `uniprot_id` FROM `protein_test` WHERE `name` = '%1' AND `start` < %2 AND 'end' > %3")
//                .arg(name,posStart,posEnd));
    query.exec(
                QString("SELECT DISTINCT `name`, `start`, `end`, `uniprot_id`, `Scan(s)`, `proteoform` FROM `protein_test`, `protein_sequence` WHERE protein_test.name = '%1' AND protein_test.start < %2 AND protein_test.end > %3 AND protein_test.uniprot_id = protein_sequence.`Protein accession` ")
                .arg(name,posStart,posEnd));

    QString result;
    while(query.next())
    {
//         result = result + query.value(0).toString() + '\n';
        result = result + query.value(0).toString() + '\t' +
                query.value(1).toString() + '\t' +
                query.value(2).toString() + '\t' +
                query.value(3).toString() + '\t' +
                query.value(4).toString() + '\t' +
                query.value(5).toString() + '\n';
    }
    return result;
}

