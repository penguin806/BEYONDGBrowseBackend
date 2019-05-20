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

#include <iostream>
using namespace std;
int longestCommonSubstring_n2_n2(const string& str1, const string& str2)
{
    size_t size1 = str1.size();
    size_t size2 = str2.size();
    if (size1 == 0 || size2 == 0) return 0;

    vector<vector<int> > table(size1, vector<int>(size2, 0));
    // the start position of substring in original string
    int start1 = -1;
    int start2 = -1;
    // the longest length of common substring
    int longest = 0;

    // record how many comparisons the solution did;
    // it can be used to know which algorithm is better
    int comparisons = 0;
    for (int j = 0; j < size2; ++j)
    {
        ++comparisons;
        table[0][j] = (str1[0] == str2[j] ? 1 :0);
    }

    for (int i = 1; i < size1; ++i)
    {
        ++comparisons;
        table[i][0] = (str1[i] == str2[0] ? 1 :0);

        for (int j = 1; j < size2; ++j)
        {
            ++comparisons;
            if (str1[i] == str2[j])
            {
                table[i][j] = table[i-1][j-1]+1;
            }
        }
    }

    for (int i = 0; i < size1; ++i)
    {
        for (int j = 0; j < size2; ++j)
        {
            if (longest < table[i][j])
            {
                longest = table[i][j];
                start1 = i-longest+1;
                start2 = j-longest+1;
            }
        }
    }

    cout<< "(first, second, comparisions) = ("
        << start1 << ", " << start2 << ", " << comparisons
        << ")" << endl;

    return longest;
}


QString ProteinQuery::queryProteinUniprotId(QString name, QString posStart, QString posEnd)
{
    QSqlQuery query;
//    query.exec(
//                QString("SELECT `uniprot_id` FROM `protein_test` WHERE `name` = '%1' AND `start` < %2 AND 'end' > %3")
//                .arg(name,posStart,posEnd));
//    QString queryString = QString("SELECT DISTINCT `name`, `start`, `end`, `uniprot_id`, `Scan(s)`, `proteoform` FROM `protein_test`, `protein_sequence` WHERE protein_test.name = '%1' AND protein_test.start < %2 AND protein_test.end > %3 AND protein_test.uniprot_id = protein_sequence.`Protein accession` ")
//            .arg(name,posStart,posEnd);

    QString queryString = QString(
        "SELECT DISTINCT\
            `name`,\
            `start`,\
            `end`,\
            `uniprot_id`,\
            `Scan(s)`,\
            `proteoform`\
        FROM\
            (\
            SELECT\
                `name`,\
                `start`,\
                `end`,\
                `uniprot_id`,\
                COUNT(DISTINCT `uniprot_id`)\
            FROM\
                `protein_test`\
            WHERE\
                `name` = '%1' AND `start` < '%2' AND `end` > '%3'\
        ) AS `protein_info`,\
        `protein_sequence`\
        WHERE\
            `uniprot_id` = `Protein accession`"
    ).arg(name, posStart, posEnd);

    qDebug() << "Query: " + queryString;
    query.exec(queryString);

    QString result;
    string proteinSequences;
    while(query.next())
    {
        result = result + query.value(0).toString() + '\t' +
                query.value(1).toString() + '\t' +
                query.value(2).toString() + '\t' +
                query.value(3).toString() + '\t' +
                query.value(4).toString() + '\t' +
                query.value(5).toString() + '\n';
        proteinSequences += query.value(5).toString().toStdString();
    }

    //string sequenceToMap = "AARKSAPATGGVKKPHYRPGTVAL";
    string sequenceToMap = "(EIRRYQKSTELLIRKLPFQRLVREIAQDFKTDLRFQSSAV)[Methyl]MALQEASEAYLVGLFEDTNLCAIHAKRVTIMPKDI";

    qDebug() << "\n\n\n" << "proteinSequences: ";
    cout  << proteinSequences << "\n\n\n" << sequenceToMap << "\n\n\n" ;
    qDebug() << "\n\n\n" <<
                    longestCommonSubstring_n2_n2(proteinSequences, sequenceToMap)
            << "\n\n\n";


    return result;
}

