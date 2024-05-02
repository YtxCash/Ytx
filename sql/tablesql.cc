#include "tablesql.h"

#include <QSqlError>
#include <QSqlQuery>

#include "component/constvalue.h"
#include "global/sqlconnection.h"
#include "global/transactionpool.h"
#include "global/transpool.h"

bool TableSql::RRemoveMulti(int node_id)
{
    auto node_trans { RelatedNodeAndTrans(node_id) };
    auto trans_id_list { node_trans.values() };

    // begin deal with database
    QSqlQuery query(*db_);

    QStringList list {};
    for (const int& id : trans_id_list)
        list.append(QString::number(id));

    auto part = QString("UPDATE %1 "
                        "SET removed = 1 "
                        "WHERE id IN (%2) ")
                    .arg(info_->transaction, list.join(", "));

    query.prepare(part);
    if (!query.exec()) {
        qWarning() << "Failed to remove record in transaction table" << query.lastError().text();
        return false;
    }
    // end deal with database

    emit SFreeView(node_id);
    emit SRemoveNode(node_id);
    emit SRemoveMulti(node_trans);
    emit SUpdateMultiTotal(node_trans.uniqueKeys());

    // begin deal with transaction hash
    for (int i = 0; i != trans_id_list.size(); ++i) {
        int trans_id { trans_id_list.at(i) };

        TransactionPool::Instance().Recycle(transaction_hash_.take(trans_id));
    }
    // end deal with transaction hash

    return true;
}

bool TableSql::RReplaceMulti(int old_node_id, int new_node_id)
{
    auto node_trans { RelatedNodeAndTrans(old_node_id) };
    bool free { !node_trans.contains(new_node_id) };

    node_trans.remove(new_node_id);
    if (node_trans.isEmpty())
        return true;

    // begin deal with transaction hash
    for (auto& transaction : std::as_const(transaction_hash_)) {
        if (transaction->lhs_node == old_node_id && transaction->rhs_node != new_node_id)
            transaction->lhs_node = new_node_id;

        if (transaction->rhs_node == old_node_id && transaction->lhs_node != new_node_id)
            transaction->rhs_node = new_node_id;
    }
    // end deal with transaction hash

    // begin deal with database
    QSqlQuery query(*db_);
    auto part = QString("UPDATE %1 SET "
                        "lhs_node = CASE "
                        "WHEN lhs_node = :old_node_id AND rhs_node != :new_node_id THEN :new_node_id "
                        "ELSE lhs_node END, "
                        "rhs_node = CASE "
                        "WHEN rhs_node = :old_node_id AND lhs_node != :new_node_id THEN :new_node_id "
                        "ELSE rhs_node END ")
                    .arg(info_->transaction);

    query.prepare(part);
    query.bindValue(":new_node_id", new_node_id);
    query.bindValue(":old_node_id", old_node_id);
    if (!query.exec()) {
        qWarning() << "Error in replace node setp" << query.lastError().text();
        return false;
    }
    // end deal with database

    if (free) {
        emit SFreeView(old_node_id);
        emit SRemoveNode(old_node_id);
    }

    emit SMoveMulti(info_->section, old_node_id, new_node_id, node_trans.values());
    emit SUpdateMultiTotal(QList { old_node_id, new_node_id });

    return true;
}

TableSql::TableSql(QObject* parent)
    : QObject(parent)
{
}

void TableSql::SetInfo(const Info* info)
{
    info_ = info;
    db_ = SqlConnection::Instance().Allocate(info->section);
}

SPTransList TableSql::TransList(int node_id)
{
    QSqlQuery query(*db_);
    query.setForwardOnly(true);

    auto part
        = QString("SELECT id, lhs_node, lhs_ratio, lhs_debit, lhs_credit, sender_receiver, section_marker, rhs_node, rhs_ratio, rhs_debit, rhs_credit, state, "
                  "description, code, document, date_time "
                  "FROM %1 "
                  "WHERE (lhs_node = :node_id OR rhs_node = :node_id) AND removed = 0 ")
              .arg(info_->transaction);

    query.prepare(part);
    query.bindValue(":node_id", node_id);

    if (!query.exec()) {
        qWarning() << "Error in Construct Table" << query.lastError().text();
        return SPTransList();
    }

    return QueryList(node_id, query);
}

void TableSql::Convert(CSPTransaction& transaction, SPTrans& trans, bool left)
{
    trans->id = &transaction->id;
    trans->state = &transaction->state;
    trans->date_time = &transaction->date_time;
    trans->code = &transaction->code;
    trans->document = &transaction->document;
    trans->description = &transaction->description;
    trans->location = &transaction->location;
    trans->transport = &transaction->transport;

    if (left) {
        trans->node = &transaction->lhs_node;
        trans->ratio = &transaction->lhs_ratio;
        trans->debit = &transaction->lhs_debit;
        trans->credit = &transaction->lhs_credit;

        trans->related_node = &transaction->rhs_node;
        trans->related_ratio = &transaction->rhs_ratio;
        trans->related_debit = &transaction->rhs_debit;
        trans->related_credit = &transaction->rhs_credit;

        return;
    }

    trans->node = &transaction->rhs_node;
    trans->ratio = &transaction->rhs_ratio;
    trans->debit = &transaction->rhs_debit;
    trans->credit = &transaction->rhs_credit;

    trans->related_node = &transaction->lhs_node;
    trans->related_ratio = &transaction->lhs_ratio;
    trans->related_debit = &transaction->lhs_debit;
    trans->related_credit = &transaction->lhs_credit;
}

bool TableSql::Insert(CSPTrans& trans)
{
    QSqlQuery query(*db_);
    auto part = QString("INSERT INTO %1 "
                        "(date_time, lhs_node, lhs_ratio, lhs_debit, lhs_credit, sender_receiver, section_marker, rhs_node, rhs_ratio, rhs_debit, rhs_credit, "
                        "state, description, code, "
                        "document) "
                        "VALUES "
                        "(:date_time, :lhs_node, :lhs_ratio, :lhs_debit, :lhs_credit, :sender_receiver, :section_marker, :rhs_node, :rhs_ratio, :rhs_debit, "
                        ":rhs_credit, :state, "
                        ":description, :code, :document) ")
                    .arg(info_->transaction);

    query.prepare(part);
    query.bindValue(":date_time", *trans->date_time);
    query.bindValue(":lhs_node", *trans->node);
    query.bindValue(":lhs_ratio", *trans->ratio);
    query.bindValue(":lhs_debit", *trans->debit);
    query.bindValue(":lhs_credit", *trans->credit);
    query.bindValue(":rhs_node", *trans->related_node);
    query.bindValue(":sender_receiver", *trans->transport);
    query.bindValue(":section_marker", trans->location->join(SEMICOLON));
    query.bindValue(":rhs_ratio", *trans->related_ratio);
    query.bindValue(":rhs_debit", *trans->related_debit);
    query.bindValue(":rhs_credit", *trans->related_credit);
    query.bindValue(":state", *trans->state);
    query.bindValue(":description", *trans->description);
    query.bindValue(":code", *trans->code);
    query.bindValue(":document", trans->document->join(SEMICOLON));

    if (!query.exec()) {
        qWarning() << "Failed to insert record in transaction table" << query.lastError().text();
        return false;
    }

    *trans->id = query.lastInsertId().toInt();
    transaction_hash_.insert(*trans->id, last_insert_transaction_);
    return true;
}

bool TableSql::Delete(int trans_id)
{
    QSqlQuery query(*db_);
    auto part = QString("UPDATE %1 "
                        "SET removed = 1 "
                        "WHERE id = :trans_id ")
                    .arg(info_->transaction);

    query.prepare(part);
    query.bindValue(":trans_id", trans_id);
    if (!query.exec()) {
        qWarning() << "Failed to remove record in transaction table" << query.lastError().text();
        return false;
    }

    TransactionPool::Instance().Recycle(transaction_hash_.take(trans_id));
    return true;
}

bool TableSql::Update(int trans_id)
{
    QSqlQuery query(*db_);
    auto transaction { transaction_hash_.value(trans_id) };

    auto part = QString("UPDATE %1 SET "
                        "lhs_node = :lhs_node, "
                        "lhs_ratio = :lhs_ratio, "
                        "lhs_debit = :lhs_debit, "
                        "lhs_credit = :lhs_credit, "
                        "rhs_node = :rhs_node, "
                        "rhs_ratio = :rhs_ratio, "
                        "rhs_debit = :rhs_debit, "
                        "rhs_credit = :rhs_credit "
                        "WHERE id = :id ")
                    .arg(info_->transaction);

    query.prepare(part);
    query.bindValue(":lhs_node", transaction->lhs_node);
    query.bindValue(":lhs_ratio", transaction->lhs_ratio);
    query.bindValue(":lhs_debit", transaction->lhs_debit);
    query.bindValue(":lhs_credit", transaction->lhs_credit);
    query.bindValue(":rhs_node", transaction->rhs_node);
    query.bindValue(":rhs_ratio", transaction->rhs_ratio);
    query.bindValue(":rhs_debit", transaction->rhs_debit);
    query.bindValue(":rhs_credit", transaction->rhs_credit);
    query.bindValue(":id", trans_id);

    if (!query.exec()) {
        qWarning() << "Failed to update record in transaction table 1st " << query.lastError().text();
        return false;
    }

    return true;
}

bool TableSql::Update(CString& column, const QVariant& value, int trans_id)
{
    QSqlQuery query(*db_);

    auto part = QString("UPDATE %1 "
                        "SET %2 = :value "
                        "WHERE id = :trans_id ")
                    .arg(info_->transaction, column);

    query.prepare(part);
    query.bindValue(":value", value);
    query.bindValue(":trans_id", trans_id);

    if (!query.exec()) {
        qWarning() << "Failed to update record in transaction table 2nd " << query.lastError().text();
        return false;
    }

    return true;
}

bool TableSql::Update(CString& column, const QVariant& value, Check state)
{
    QSqlQuery query(*db_);

    auto part = QString("UPDATE %1 "
                        "SET %2 = :value ")
                    .arg(info_->transaction, column);

    if (state == Check::kReverse)
        part = QString("UPDATE %1 "
                       "SET %2 = NOT %2 ")
                   .arg(info_->transaction, column);

    query.prepare(part);
    query.bindValue(":value", value);

    if (!query.exec()) {
        qWarning() << "Failed to update record in transaction table " << query.lastError().text();
        return false;
    }

    return true;
}

SPTransList TableSql::TransList(int node_id, const QList<int>& trans_id_list)
{
    QSqlQuery query(*db_);
    query.setForwardOnly(true);

    QStringList list {};

    for (const int& id : trans_id_list)
        list.append(QString::number(id));

    auto part
        = QString("SELECT id, lhs_node, lhs_ratio, lhs_debit, lhs_credit, sender_receiver, section_marker, rhs_node, rhs_ratio, rhs_debit, rhs_credit, state, "
                  "description, code, document, date_time "
                  "FROM %1 "
                  "WHERE id IN (%2) AND (lhs_node = :node_id OR rhs_node = :node_id) AND removed = 0 ")
              .arg(info_->transaction, list.join(", "));

    query.prepare(part);
    query.bindValue(":node_id", node_id);

    if (!query.exec()) {
        qWarning() << "Error in ConstructTable 1st" << query.lastError().text();
        return SPTransList();
    }

    return QueryList(node_id, query);
}

SPTransaction TableSql::Transaction(int trans_id)
{
    QSqlQuery query(*db_);
    query.setForwardOnly(true);

    auto part
        = QString("SELECT id, lhs_node, lhs_ratio, lhs_debit, lhs_credit, sender_receiver, section_marker, rhs_node, rhs_ratio, rhs_debit, rhs_credit, state, "
                  "description, code, document, date_time "
                  "FROM %1 "
                  "WHERE id = :trans_id AND removed = 0 ")
              .arg(info_->transaction);

    query.prepare(part);
    query.bindValue(":trans_id", trans_id);

    if (!query.exec()) {
        qWarning() << "Error in ConstructTable 1st" << query.lastError().text();
        return SPTransaction();
    }

    return QueryTransaction(trans_id, query);
}

SPTrans TableSql::AllocateTrans()
{
    last_insert_transaction_ = TransactionPool::Instance().Allocate();
    auto trans { TransPool::Instance().Allocate() };

    Convert(last_insert_transaction_, trans, true);
    return trans;
}

QMultiHash<int, int> TableSql::RelatedNodeAndTrans(int node_id) const
{
    QSqlQuery query(*db_);
    query.setForwardOnly(true);

    auto part = QString("SELECT lhs_node, id FROM %1 "
                        "WHERE rhs_node = :node_id AND removed = 0 "
                        "UNION ALL "
                        "SELECT rhs_node, id FROM %1 "
                        "WHERE lhs_node = :node_id AND removed = 0 ")
                    .arg(info_->transaction);

    query.prepare(part);
    query.bindValue(":node_id", node_id);

    if (!query.exec()) {
        qWarning() << "Error in RelatedNodeAndTrans 1st" << query.lastError().text();
    }

    QMultiHash<int, int> hash {};
    int related_node_id {};
    int trans_id {};

    while (query.next()) {
        related_node_id = query.value("lhs_node").toInt();
        trans_id = query.value("id").toInt();
        hash.insert(related_node_id, trans_id);
    }

    return hash;
}

SPTransList TableSql::QueryList(int node_id, QSqlQuery& query)
{
    SPTrans trans {};
    SPTransList trans_list {};
    SPTransaction transaction {};
    int id {};

    while (query.next()) {
        id = query.value("id").toInt();
        trans = TransPool::Instance().Allocate();

        if (transaction_hash_.contains(id)) {
            transaction = transaction_hash_.value(id);
            Convert(transaction, trans, node_id == transaction->lhs_node);
            trans_list.emplaceBack(trans);
            continue;
        }

        transaction = TransactionPool::Instance().Allocate();
        transaction->id = id;

        transaction->lhs_node = query.value("lhs_node").toInt();
        transaction->lhs_ratio = query.value("lhs_ratio").toDouble();
        transaction->lhs_debit = query.value("lhs_debit").toDouble();
        transaction->lhs_credit = query.value("lhs_credit").toDouble();

        transaction->rhs_node = query.value("rhs_node").toInt();
        transaction->rhs_ratio = query.value("rhs_ratio").toDouble();
        transaction->rhs_debit = query.value("rhs_debit").toDouble();
        transaction->rhs_credit = query.value("rhs_credit").toDouble();

        transaction->code = query.value("code").toString();
        transaction->description = query.value("description").toString();
        transaction->document = query.value("document").toString().split(SEMICOLON, Qt::SkipEmptyParts);
        transaction->date_time = query.value("date_time").toString();
        transaction->state = query.value("state").toBool();
        transaction->transport = query.value("sender_receiver").toInt();
        transaction->location = query.value("section_marker").toString().split(SEMICOLON, Qt::SkipEmptyParts);

        transaction_hash_.insert(id, transaction);
        Convert(transaction, trans, node_id == transaction->lhs_node);
        trans_list.emplaceBack(trans);
    }

    return trans_list;
}

SPTransaction TableSql::QueryTransaction(int trans_id, QSqlQuery& query)
{
    if (transaction_hash_.contains(trans_id))
        return transaction_hash_.value(trans_id);

    SPTransaction transaction { TransactionPool::Instance().Allocate() };

    query.next();

    transaction->id = trans_id;

    transaction->lhs_node = query.value("lhs_node").toInt();
    transaction->lhs_ratio = query.value("lhs_ratio").toDouble();
    transaction->lhs_debit = query.value("lhs_debit").toDouble();
    transaction->lhs_credit = query.value("lhs_credit").toDouble();

    transaction->rhs_node = query.value("rhs_node").toInt();
    transaction->rhs_ratio = query.value("rhs_ratio").toDouble();
    transaction->rhs_debit = query.value("rhs_debit").toDouble();
    transaction->rhs_credit = query.value("rhs_credit").toDouble();

    transaction->code = query.value("code").toString();
    transaction->description = query.value("description").toString();
    transaction->document = query.value("document").toString().split(SEMICOLON, Qt::SkipEmptyParts);
    transaction->date_time = query.value("date_time").toString();
    transaction->state = query.value("state").toBool();
    transaction->transport = query.value("sender_receiver").toInt();
    transaction->location = query.value("section_marker").toString().split(SEMICOLON, Qt::SkipEmptyParts);

    return transaction;
}
