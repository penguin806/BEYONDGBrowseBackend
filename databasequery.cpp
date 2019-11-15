#include "databasequery.h"
#include <QSqlQuery>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
//#include <QJsonDocument>

DatabaseQuery::DatabaseQuery(QSqlDatabase databaseConnection)
{
    this->databaseConnection = databaseConnection;
}

// 取回位置区间对应的蛋白质变体信息
QJsonArray DatabaseQuery::queryProteinBySequenceRegion(QString name, QString posStart, QString posEnd)
{
    if(!this->databaseConnection.isOpen())
    {
        throw QString("ERROR_DATABASE_CLOSED");
    }
    QSqlQuery query(this->databaseConnection);

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
    `strand`,\
    `uniprot_id`,\
    `Scan(s)`,\
    `ions`,\
    `proteoform`\
FROM\
    (\
    SELECT\
        `start`,\
        `end`,\
        `strand`,\
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
    qDebug() << "[Info] queryProteinBySequenceRegion: "
             << name << posStart << posEnd << bQueryResult << query.size();

    QJsonArray recordArray;
    while(query.next())
    {
        QJsonObject oneLineRecord;
        oneLineRecord.insert("_start", query.value("start").toString());
        oneLineRecord.insert("end", query.value("end").toString());
        oneLineRecord.insert("strand", query.value("strand").toString());
        oneLineRecord.insert("uniprot_id", query.value("uniprot_id").toString());
        oneLineRecord.insert("scanId", query.value("Scan(s)").toString());
        oneLineRecord.insert("sequence", query.value("proteoform").toString());

        // Convert 3891.12898~4335.73;3990.18703~1990.40;  =>  [3891.12898, 3990.18703], [4335.73, 1990.40]
        QStringList ionsList = query.value("ions").toString().split(';',QString::SkipEmptyParts);
        QJsonArray arrMSScanMassArray;
        QJsonArray arrMSScanPeakAundance;
        QJsonArray arrIonsNum;
        for (QString ionsItem : ionsList) {
            QStringList massAndPeakAndIons = ionsItem.split('~', QString::SkipEmptyParts);
            arrMSScanMassArray.append(massAndPeakAndIons.at(0));
            arrMSScanPeakAundance.append(massAndPeakAndIons.at(1));
            arrIonsNum.append(massAndPeakAndIons.at(2).toInt());
        }
        oneLineRecord.insert("arrMSScanMassArray", arrMSScanMassArray);
        oneLineRecord.insert("arrMSScanPeakAundance", arrMSScanPeakAundance);
        oneLineRecord.insert("arrIonsNum", arrIonsNum);

        recordArray.push_back(oneLineRecord);
    }

//    QJsonDocument jsonDocument(recordArray);
//    return QString(jsonDocument.toJson(QJsonDocument::Indented));
    return recordArray;
}

// 通过蛋白质ID查询范围
QJsonArray DatabaseQuery::queryRegionByProteinId(QString proteinName)
{
    if(!this->databaseConnection.isOpen())
    {
        throw QString("ERROR_DATABASE_CLOSED");
    }
    QSqlQuery query(this->databaseConnection);

    // SELECT `name`,`start`,`end` FROM `protein_annotation` WHERE `ensembl_id` = 'ENSP00000000233'
    QString queryString =
            QString("SELECT `name`,`start`,`end` FROM `protein_annotation` WHERE `ensembl_id` = '%1' OR `uniprot_id` = '%1'")
            .arg(proteinName);

    bool bQueryResult = query.exec(queryString);
    qDebug() << "[Info] queryRegionByProteinId: "
             << proteinName << bQueryResult << query.size();

    QJsonArray recordArray;
    while(query.next())
    {
        QJsonObject oneLineRecord;
        oneLineRecord.insert("name",query.value("name").toString());
        oneLineRecord.insert("_start",query.value("start").toString());
        oneLineRecord.insert("end",query.value("end").toString());

        recordArray.push_back(oneLineRecord);
    }

//    QJsonDocument jsonDocument(recordArray);
//    return QString(jsonDocument.toJson(QJsonDocument::Indented));
    return recordArray;
}

// 获取特定范围内所有注释
QJsonArray DatabaseQuery::queryAnnotationBySequenceRegion(
        QString name, QString posStart, QString posEnd
)
{
    if(!this->databaseConnection.isOpen())
    {
        throw QString("ERROR_DATABASE_CLOSED");
    }
    QSqlQuery query(this->databaseConnection);

//    SELECT
//        `name`,
//        `position`,
//        `time`,
//        `contents`
//    FROM
//        `protein_comments`
//    WHERE
//        `name` = 'chr1' AND `position` > 149813250 AND `position` < 149813297
//    ORDER BY `time` DESC

//    name 	position 	time 	contents
//    chr1 	149813270 	2019-07-28 17:49:36 	[{}, {}]
    QString queryString = QString(
                "SELECT `name`, `position`, `time`, `contents` FROM `protein_comments` WHERE `name` = '%1' AND `position` >= %2 AND `position` <= %3 ORDER BY `time` DESC")
            .arg(name, posStart, posEnd);

    bool bQueryResult = query.exec(queryString);
    qDebug() << "[Info] queryAnnotationBySequenceRegion: "
             << name << posStart << posEnd << bQueryResult << query.size();

    QJsonArray recordArray;
    while(query.next())
    {
        QJsonObject oneLineRecord;
        oneLineRecord.insert("name",query.value("name").toString());
        oneLineRecord.insert("position",query.value("position").toString());
        oneLineRecord.insert("time",query.value("time").toString());
        oneLineRecord.insert("contents",query.value("contents").toString());

        recordArray.push_back(oneLineRecord);
    }

    return recordArray;
}

// 在特定位置插入注释
bool DatabaseQuery::insertSequenceAnnotationAtSpecificPosition(
        qint32 id, QString name, qint32 position,
        QString time, QString contents
)
{
    if(!this->databaseConnection.isOpen())
    {
        throw QString("ERROR_DATABASE_CLOSED");
    }
    QSqlQuery query(this->databaseConnection);
//    INSERT INTO `protein_comments`(
//        `name`,
//        `position`,
//        `time`,
//        `contents`
//    )
//    VALUES(:name, :position, :time, :contents)
//    ON DUPLICATE KEY
//    UPDATE
//        `name` = :name,
//        `position` = :position,
//        `time` = :time,
//        `contents` = :contents
    query.prepare("INSERT INTO `protein_comments`(`name`,`position`,`time`,`contents`) VALUES(:name, :position, :time, :contents) ON DUPLICATE KEY UPDATE `name` = :name,`position` = :position,`time` = :time,`contents` = :contents");
    query.bindValue(":name", name);
    query.bindValue(":position", position);
    query.bindValue(":time", time);
    query.bindValue(":contents", contents);
    bool bQueryResult = query.exec();
    qDebug() << "[Info] insertSequenceAnnotationAtSpecificPosition: "
             << name << position << time << bQueryResult << query.numRowsAffected();
    if(query.numRowsAffected() == 1)
    {
        return true;
    }
    else
    {
        return false;
    }
}

// 查询数据集列表
QJsonArray DatabaseQuery::queryDatasetsList()
{
    if(!this->databaseConnection.isOpen())
    {
        throw QString("ERROR_DATABASE_CLOSED");
    }
    QSqlQuery query(this->databaseConnection);

    QString queryString =
            QString("SELECT `id`, `dataset_name` FROM `dataset_catalog`");

    bool bQueryResult = query.exec(queryString);
    qDebug() << "[Info] queryDatasetsList: " <<
                bQueryResult << query.size();

    QJsonArray recordArray;
    while(query.next())
    {
        QJsonObject oneLineRecord;
        oneLineRecord.insert("id",query.value("id").toString());
        oneLineRecord.insert("dataset_name",query.value("dataset_name").toString());

        recordArray.push_back(oneLineRecord);
    }

    return recordArray;
}
