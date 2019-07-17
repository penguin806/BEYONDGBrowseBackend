#include "proteinquery.h"

#include <QtSql>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

ProteinQuery::ProteinQuery()
{
    //this->serverAddrress = "server-hk1.xuefeng.space";
    this->serverAddrress = "localhost";
    this->databaseName = "snow_db201905";
    this->username = "snow_db201905";
    this->password = "snow_db201905";
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

//int longestCommonSubstring_n2_n2(const string& str1, const string& str2)
//{
//    size_t size1 = str1.size();
//    size_t size2 = str2.size();
//    if (size1 == 0 || size2 == 0) return 0;

//    vector<vector<int> > table(size1, vector<int>(size2, 0));
//    // the start position of substring in original string
//    int start1 = -1;
//    int start2 = -1;
//    // the longest length of common substring
//    int longest = 0;

//    // record how many comparisons the solution did;
//    // it can be used to know which algorithm is better
//    int comparisons = 0;
//    for (int j = 0; j < size2; ++j)
//    {
//        ++comparisons;
//        table[0][j] = (str1[0] == str2[j] ? 1 :0);
//    }

//    for (int i = 1; i < size1; ++i)
//    {
//        ++comparisons;
//        table[i][0] = (str1[i] == str2[0] ? 1 :0);

//        for (int j = 1; j < size2; ++j)
//        {
//            ++comparisons;
//            if (str1[i] == str2[j])
//            {
//                table[i][j] = table[i-1][j-1]+1;
//            }
//        }
//    }

//    for (int i = 0; i < size1; ++i)
//    {
//        for (int j = 0; j < size2; ++j)
//        {
//            if (longest < table[i][j])
//            {
//                longest = table[i][j];
//                start1 = i-longest+1;
//                start2 = j-longest+1;
//            }
//        }
//    }

//    cout<< "(first, second, comparisions) = ("
//        << start1 << ", " << start2 << ", " << comparisons
//        << ")" << endl;

//    return longest;
//}


QString ProteinQuery::queryProteinByReferenceSequenceRegion(QString name, QString posStart, QString posEnd)
{
    QSqlQuery query;
//    query.exec(
//                QString("SELECT `uniprot_id` FROM `protein_test` WHERE `name` = '%1' AND `start` < %2 AND 'end' > %3")
//                .arg(name,posStart,posEnd));
//    QString queryString = QString("SELECT DISTINCT `name`, `start`, `end`, `uniprot_id`, `Scan(s)`, `proteoform` FROM `protein_test`, `protein_sequence` WHERE protein_test.name = '%1' AND protein_test.start < %2 AND protein_test.end > %3 AND protein_test.uniprot_id = protein_sequence.`Protein accession` ")
//            .arg(name,posStart,posEnd);

// 2019-07-11 SQL:
// SELECT
//     `start`,
//     `end`,
//     `uniprot_id`,
//     `Scan(s)`,
//     `ions`,
//     `proteoform`
// FROM
//     (
//     SELECT
//         `start`,
//         `end`,
//         `uniprot_id`
//     FROM
//         `protein_annotation`
//     WHERE
//         (
//             `name` = 'chr1' AND `start` >= '1' AND `end` <= '600000000'
//         ) OR(
//             `name` = 'chr1' AND `start` < '1' AND `end` > '1'
//         ) OR(
//             `name` = 'chr1' AND `start` < '600000000' AND `end` > '600000000'
//         ) OR(
//             `name` = 'chr1' AND `start` < '1' AND `end` > '600000000'
//         )
//     GROUP BY
//         `uniprot_id`
// ) AS `id_list`,
// `protein_sequence`,
// `protein_scan`
// WHERE
//     `uniprot_id` = `Protein accession` AND `Scan(s)` = `scan_id`
// ORDER BY
//     `start`,
//     `Scan(s)`


//  2019-06-03 SQL:
//
//    SELECT
//        `start`,
//        `end`,
//        `uniprot_id`,
//        `Scan(s)`,
//        `proteoform`
//    FROM
//        (
//        SELECT
//            `start`,
//            `end`,
//            `uniprot_id`
//        FROM
//            `protein_annotation`
//        WHERE
//            (
//                `name` = 'chr1' AND `start` >= '1' AND `end` <= '600000000'
//            ) OR(
//                `name` = 'chr1' AND `start` < '1' AND `end` > '1'
//            ) OR(
//                `name` = 'chr1' AND `start` < '600000000' AND `end` > '600000000'
//            ) OR(
//                `name` = 'chr1' AND `start` < '1' AND `end` > '600000000'
//            )
//        GROUP BY
//            `uniprot_id`
//    ) AS `id_list`,
//    `protein_sequence`
//    WHERE
//        `uniprot_id` = `Protein accession`
//    ORDER BY
//        `start`,
//        `Scan(s)`

// Test Query Result:
// start        end         uniprot_id	Scan(s) proteoform
// 149813271 	149813681 	H32_HUMAN 	912 	A.TKAARKSAPATGGVKKPHRYRPGTVALREIRRYQKST(ELLIRKLPFQ...
// 149813271 	149813681 	H32_HUMAN 	952 	T.(KQTARKSTGGKAPRKQLATKAARKSAPATGGVKKPH)[Phospho;P...
// 149813271 	149813681 	H32_HUMAN 	956 	T.(KQTARKSTGGKAPRK)[Phospho](QLATKAARKSAPATGGVKKPH...
// 149813271 	149813681 	H32_HUMAN 	962 	R.(TKQTARK)[Dimethyl;Acetyl]STGGKAPRKQLAT(KAAR)[Ac...
// 149813271 	149813681 	H32_HUMAN 	978 	R.(TKQTARKSTGGKA)[Methyl;Acetyl;Acetyl]PRKQLATKAAR...
// 149813271 	149813681 	H32_HUMAN 	998 	M.ARTKQTARKSTGGKAPRKQLATKAARK(SAPATGGVK)[Dimethyl]...
// 149813271 	149813681 	H32_HUMAN 	1020 	R.(TKQTARKSTGGKAPRKQLATKAARKSAPATGGVKKPHRYRPGT)[Ac...
// 149813271 	149813681 	H32_HUMAN 	1036 	A.(RTKQTARKSTGGKAPRKQLATKAARKSAPATG)[Acetyl]GVKKPH...
// 149813271 	149813681 	H32_HUMAN 	1041 	M.ARTKQTA(RKSTG)[Acetyl](GKAPRK)[Acetyl]QLAT(KAARK...

    QString queryString = QString(
"\
SELECT\
    `start`,\
    `end`,\
    `uniprot_id`,\
    `Scan(s)`,\
    `ions`,\
    `proteoform`\
FROM\
    (\
    SELECT\
        `start`,\
        `end`,\
        `uniprot_id`\
    FROM\
        `protein_annotation`\
    WHERE\
        (\
            `name` = '%1' AND `start` >= '%2' AND `end` <= '%3'\
        ) OR(\
            `name` = '%1' AND `start` < '%2' AND `end` > '%2'\
        ) OR(\
            `name` = '%1' AND `start` < '%3' AND `end` > '%3'\
        ) OR(\
            `name` = '%1' AND `start` < '%2' AND `end` > '%3'\
        )\
    GROUP BY\
        `uniprot_id`\
) AS `id_list`,\
`protein_sequence`,\
`protein_scan` \
WHERE\
    `uniprot_id` = `Protein accession` AND `Scan(s)` = `scan_id`\
ORDER BY\
    `start`,\
    `Scan(s)`\
"
    ).arg(
                name, posStart, posEnd
         );


    bool bQueryResult = query.exec(queryString);
    qDebug() << "Query: " << name << posStart << posEnd << bQueryResult;

    QJsonArray recordArray;
    while(query.next())
    {
        QJsonObject oneLineRecord;
        oneLineRecord.insert("_start",query.value("start").toString());
        oneLineRecord.insert("end",query.value("end").toString());
        oneLineRecord.insert("uniprot_id",query.value("uniprot_id").toString());
        oneLineRecord.insert("scanId",query.value("Scan(s)").toString());
        oneLineRecord.insert("sequence",query.value("proteoform").toString());

        // Convert 3891.12898~4335.73;3990.18703~1990.40;  =>  [3891.12898, 3990.18703], [4335.73, 1990.40]
        QStringList ionsList = query.value("ions").toString().split(';',QString::SkipEmptyParts);
        QJsonArray arrMSScanMassArray;
        QJsonArray arrMSScanPeakAundance;
        for (QString ionsItem : ionsList) {
            QStringList massAndPeak = ionsItem.split('~', QString::SkipEmptyParts);
            arrMSScanMassArray.append(massAndPeak.at(0));
            arrMSScanPeakAundance.append(massAndPeak.at(1));
        }
        oneLineRecord.insert("arrMSScanMassArray", arrMSScanMassArray);
        oneLineRecord.insert("arrMSScanPeakAundance", arrMSScanPeakAundance);

        recordArray.push_back(oneLineRecord);
    }

    QJsonDocument jsonDocument(recordArray);
    return QString(jsonDocument.toJson(QJsonDocument::Indented));
}

QString ProteinQuery::queryRegionByProteinId(QString proteinName)
{
    QSqlQuery query;
    // SELECT `name`,`start`,`end` FROM `protein_annotation` WHERE `ensembl_id` = 'ENSP00000000233'
    QString queryString =
            QString("SELECT `name`,`start`,`end` FROM `protein_annotation` WHERE `ensembl_id` = '%1' OR `uniprot_id` = '%1'")
            .arg(proteinName);

    bool bQueryResult = query.exec(queryString);
    qDebug() << "Query: " << proteinName << bQueryResult;

    QJsonArray recordArray;
    while(query.next())
    {
        QJsonObject oneLineRecord;
        oneLineRecord.insert("name",query.value("name").toString());
        oneLineRecord.insert("_start",query.value("start").toString());
        oneLineRecord.insert("end",query.value("end").toString());

        recordArray.push_back(oneLineRecord);
    }

    QJsonDocument jsonDocument(recordArray);
    return QString(jsonDocument.toJson(QJsonDocument::Indented));
}
