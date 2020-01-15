#include "databasequery.h"
#include <QSqlQuery>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QSqlError>
//#include <QJsonDocument>

DatabaseQuery::DatabaseQuery(QSqlDatabase databaseConnection)
{
    this->databaseConnection = databaseConnection;
}

// 取回位置区间对应的蛋白质变体信息
QJsonArray DatabaseQuery::queryProteinBySequenceRegion(quint16 datasetId, QString name, QString posStart, QString posEnd)
{
    if(!this->databaseConnection.isOpen())
    {
        throw QString("ERROR_DATABASE_CLOSED");
    }
    QSqlQuery query(this->databaseConnection);

// 2019-11-16 SQL:
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
//         `dataset_id` = 1 AND (
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
//     `protein_scan`.`dataset_id` = 1 AND `protein_sequence`.`dataset_id` = 1 AND `uniprot_id` = `Protein accession` AND `Scan(s)` = `scan_id`
// ORDER BY
//     `start`,
//     `Scan(s)`

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
    `ensembl_id`,\
    `Scan(s)`,\
    `ions`,\
    `proteoform`\
FROM\
    (\
    SELECT\
        `start`,\
        `end`,\
        `strand`,\
        `uniprot_id`,\
        `ensembl_id`\
    FROM\
        `protein_annotation`\
    WHERE\
        `dataset_id` = %4 AND (\
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
    `protein_scan`.`dataset_id` = %4 AND `protein_sequence`.`dataset_id` = %4 AND `uniprot_id` = `Protein accession` AND `Scan(s)` = `scan_id`\
ORDER BY\
    `start`,\
    `Scan(s)`\
"
    ).arg(
                name, posStart, posEnd, QString::number(datasetId)
         );


    bool bQueryResult = query.exec(queryString);
    qDebug() << "[Info] queryProteinBySequenceRegion: "
             << datasetId << name << posStart << posEnd << bQueryResult << query.size();

    QJsonArray recordArray;
    while(query.next())
    {
        QJsonObject oneLineRecord;
        oneLineRecord.insert("_start", query.value("start").toString());
        oneLineRecord.insert("end", query.value("end").toString());
        oneLineRecord.insert("strand", query.value("strand").toString());
        oneLineRecord.insert("uniprot_id", query.value("uniprot_id").toString());
        oneLineRecord.insert("ensembl_id", query.value("ensembl_id").toString());
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
QJsonArray DatabaseQuery::queryRegionByProteinId(quint16 datasetId, QString proteinName)
{
    if(!this->databaseConnection.isOpen())
    {
        throw QString("ERROR_DATABASE_CLOSED");
    }
    QSqlQuery query(this->databaseConnection);

    // SELECT `name`,`start`,`end` FROM `protein_annotation` WHERE `ensembl_id` = 'ENSP00000000233'
    QString queryString =
            QString("SELECT `name`,`start`,`end` FROM `protein_annotation` WHERE `dataset_id` = %1 AND (`ensembl_id` = '%2' OR `uniprot_id` = '%2')")
            .arg(QString::number(datasetId), proteinName);

    bool bQueryResult = query.exec(queryString);
    qDebug() << "[Info] queryRegionByProteinId: "
             << datasetId << proteinName << bQueryResult << query.size();

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
        quint16 datasetId, QString name, QString posStart, QString posEnd
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
//        `dataset_id` = 1 AND `name` = 'chr1' AND `position` > 149813250 AND `position` < 149813297
//    ORDER BY `time` DESC

//    name 	position 	time 	contents
//    chr1 	149813270 	2019-07-28 17:49:36 	[{}, {}]
    QString queryString = QString(
                "SELECT `name`, `position`, `time`, `contents` FROM `protein_comments` WHERE `dataset_id` = %4 AND `name` = '%1' AND `position` >= %2 AND `position` <= %3 ORDER BY `time` DESC")
            .arg(name, posStart, posEnd, QString::number(datasetId));

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
        quint16 datasetId, qint32 id, QString name, qint32 position,
        QString time, QString contents, QString authorUsername, QString remoteAddress
)
{
    if(!this->databaseConnection.isOpen())
    {
        throw QString("ERROR_DATABASE_CLOSED");
    }
    QSqlQuery query(this->databaseConnection);
//    INSERT INTO `protein_comments`(
//        `dataset_id`,
//        `name`,
//        `position`,
//        `time`,
//        `contents`
//    )
//    VALUES(:dataset_id, :name, :position, :time, :contents)
//    ON DUPLICATE KEY
//    UPDATE
//        `dataset_id` = :dataset_id,
//        `name` = :name,
//        `position` = :position,
//        `time` = :time,
//        `contents` = :contents
    query.prepare("INSERT INTO `protein_comments`(`dataset_id`,`name`,`position`,`time`,`contents`,`ipaddress`,`author`) VALUES(:dataset_id, :name, :position, :time, :contents, :ipaddress, :author) ON DUPLICATE KEY UPDATE `dataset_id` = :dataset_id,`name` = :name,`position` = :position,`time` = :time,`contents` = :contents,`ipaddress` = :ipaddress,`author` = :author");
    query.bindValue(":dataset_id", datasetId);
    query.bindValue(":name", name);
    query.bindValue(":position", position);
    query.bindValue(":time", time);
    query.bindValue(":contents", contents);
    query.bindValue(":ipaddress", remoteAddress);
    query.bindValue(":author", authorUsername);
    bool bQueryResult = query.exec();
    qDebug() << "[Info] insertSequenceAnnotationAtSpecificPosition: "
             << datasetId << name << position << time << bQueryResult << query.numRowsAffected();
    if(query.numRowsAffected() == 1)
    {
        return true;
    }
    else
    {
        qDebug() << "[Error] insertSequenceAnnotationAtSpecificPosition: "
                 << query.lastError().text();
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
        oneLineRecord.insert("id", query.value("id").toInt());
        oneLineRecord.insert("dataset_name", query.value("dataset_name").toString());

        recordArray.push_back(oneLineRecord);
    }

    return recordArray;
}

// 查询所有以proteinName开头的蛋白质ID
QJsonArray DatabaseQuery::queryProteinIdListForAutoComplete(quint16 datasetId, QString proteinName)
{
    if(!this->databaseConnection.isOpen())
    {
        throw QString("ERROR_DATABASE_CLOSED");
    }
    QSqlQuery query(this->databaseConnection);

    // SELECT DISTINCT `uniprot_id` FROM `protein_annotation` WHERE `uniprot_id` LIKE 'H32% LIMIT 100'
    query.prepare("SELECT DISTINCT `uniprot_id` FROM `protein_annotation` WHERE `uniprot_id` LIKE :proteinName LIMIT 100");
    query.bindValue(":proteinName", proteinName + '%');
    bool bQueryResult = query.exec();

    qDebug() << "[Info] queryProteinIdListForAutoComplete: "
             << datasetId << proteinName << bQueryResult << query.size();

    QJsonArray recordArray;
    while(query.next())
    {
        recordArray.push_back(query.value("uniprot_id").toString());
    }

    return recordArray;
}
