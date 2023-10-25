#include "sqlorder.h"

#include <QSqlError>
#include <QSqlQuery>

#include "component/constvalue.h"
#include "global/nodepool.h"
#include "global/transactionpool.h"
#include "global/transpool.h"

SqlOrder::SqlOrder(const Info* info, QObject* parent)
    : Sql(info, parent)
{
}

bool SqlOrder::Tree(NodeHash& node_hash)
{
    QSqlQuery query(*db_);
    query.setForwardOnly(true);

    auto part_1st = QString("SELECT name, id, first_property, employee, third_property, discount, refund, stakeholder, date_time, description, posted, branch, "
                            "mark, initial_total, final_total "
                            "FROM %1 "
                            "WHERE removed = 0 ")
                        .arg(info_->node);

    // "WHERE mark = 2 AND removed = 0 ")

    auto part_2nd = QString("SELECT ancestor, descendant "
                            "FROM %1 "
                            "WHERE distance = 1 ")
                        .arg(info_->path);

    query.prepare(part_1st);
    if (!query.exec()) {
        qWarning() << "Error in order create tree 1 setp " << query.lastError().text();
        return false;
    }

    CreateNodeHash(query, node_hash);
    query.clear();

    query.prepare(part_2nd);
    if (!query.exec()) {
        qWarning() << "Error in order create tree 2 setp " << query.lastError().text();
        return false;
    }

    SetRelationship(query, node_hash);

    return true;
}

bool SqlOrder::Insert(int parent_id, Node* node)
{
    if (!node || node->id == -1)
        return false;

    QSqlQuery query(*db_);

    auto part_1st = QString(
        "INSERT INTO %1 (name, first_property, employee, third_property, discount, refund, stakeholder, date_time, description, posted, branch, mark, "
        "initial_total, final_total ) "
        "VALUES (:name, :first_property, :employee, :third_property, :discount, :refund, :stakeholder, :date_time, :description, :posted, :branch, "
        ":mark, :initial_total, :final_total ) ")
                        .arg(info_->node);

    auto part_2nd = QString("INSERT INTO %1 (ancestor, descendant, distance) "
                            "SELECT ancestor, :node_id, distance + 1 FROM %1 "
                            "WHERE descendant = :parent "
                            "UNION ALL "
                            "SELECT :node_id, :node_id, 0 ")
                        .arg(info_->path);

    if (!DBTransaction([&]() {
            // 插入节点记录
            query.prepare(part_1st);
            query.bindValue(":name", node->name);
            query.bindValue(":first_property", node->first_property);
            query.bindValue(":employee", node->second_property);
            query.bindValue(":third_property", node->third_property);
            query.bindValue(":discount", node->fourth_property);
            query.bindValue(":refund", node->fifth_property);
            query.bindValue(":stakeholder", node->seventh_property);
            query.bindValue(":date_time", node->date_time);
            query.bindValue(":description", node->description);
            query.bindValue(":posted", node->node_rule);
            query.bindValue(":branch", node->branch);
            query.bindValue(":mark", node->unit);
            query.bindValue(":initial_total", node->initial_total);
            query.bindValue(":final_total", node->final_total);

            if (!query.exec()) {
                qWarning() << "Failed to insert order_node record: " << query.lastError().text();
                return false;
            }

            // 获取最后插入的ID
            node->id = query.lastInsertId().toInt();

            query.clear();

            // 插入节点路径记录
            query.prepare(part_2nd);
            query.bindValue(":node_id", node->id);
            query.bindValue(":parent", parent_id);

            if (!query.exec()) {
                qWarning() << "Failed to insert order_node_path record: " << query.lastError().text();
                return false;
            }

            return true;
        })) {
        qWarning() << "Failed to insert order record";
        return false;
    }

    return true;
}

void SqlOrder::LeafTotal(Node* node)
{
    if (!node || node->id == -1 || node->branch)
        return;

    QSqlQuery query(*db_);
    query.setForwardOnly(true);
    // todo!
}

SPTransList SqlOrder::TransList(int node_id)
{
    QSqlQuery query(*db_);
    query.setForwardOnly(true);

    auto part = QString("SELECT id, lhs_node, lhs_ratio, lhs_debit, lhs_credit, transport, location, rhs_node, rhs_ratio, rhs_debit, rhs_credit, state, "
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

bool SqlOrder::Insert(CSPTrans& trans)
{
    QSqlQuery query(*db_);
    auto part = QString("INSERT INTO %1 "
                        "(date_time, lhs_node, lhs_ratio, lhs_debit, lhs_credit, transport, location, rhs_node, rhs_ratio, rhs_debit, rhs_credit, "
                        "state, description, code, "
                        "document) "
                        "VALUES "
                        "(:date_time, :lhs_node, :lhs_ratio, :lhs_debit, :lhs_credit, :transport, :location, :rhs_node, :rhs_ratio, :rhs_debit, "
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
    query.bindValue(":transport", *trans->transport);
    query.bindValue(":location", trans->location->join(SEMICOLON));
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
    transaction_hash_.insert(*trans->id, last_transaction_);
    return true;
}

SPTransList SqlOrder::TransList(int node_id, const QList<int>& trans_id_list)
{
    QSqlQuery query(*db_);
    query.setForwardOnly(true);

    QStringList list {};

    for (const auto& id : trans_id_list)
        list.append(QString::number(id));

    auto part = QString("SELECT id, lhs_node, lhs_ratio, lhs_debit, lhs_credit, transport, location, rhs_node, rhs_ratio, rhs_debit, rhs_credit, state, "
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

SPTransaction SqlOrder::Transaction(int trans_id)
{
    QSqlQuery query(*db_);
    query.setForwardOnly(true);

    auto part = QString("SELECT id, lhs_node, lhs_ratio, lhs_debit, lhs_credit, transport, location, rhs_node, rhs_ratio, rhs_debit, rhs_credit, state, "
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

void SqlOrder::CreateNodeHash(QSqlQuery& query, NodeHash& node_hash)
{
    int node_id {};
    Node* node {};

    while (query.next()) {
        node = NodePool::Instance().Allocate();
        node_id = query.value("id").toInt();

        node->id = node_id;
        node->name = query.value("name").toString();
        node->first_property = query.value("first_property").toInt();
        node->second_property = query.value("employee").toInt();
        node->third_property = query.value("third_property").toDouble();
        node->fourth_property = query.value("discount").toDouble();
        node->fifth_property = query.value("refund").toBool();
        node->seventh_property = query.value("stakeholder").toBool();
        node->date_time = query.value("date_time").toString();
        node->description = query.value("description").toString();
        node->node_rule = query.value("posted").toBool();
        node->branch = query.value("branch").toBool();
        node->unit = query.value("mark").toInt();
        node->initial_total = query.value("initial_total").toDouble();
        node->final_total = query.value("final_total").toDouble();
        node_hash.insert(node_id, node);
    }
}

SPTransList SqlOrder::QueryList(int node_id, QSqlQuery& query)
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
        transaction->transport = query.value("transport").toInt();
        transaction->location = query.value("location").toString().split(SEMICOLON, Qt::SkipEmptyParts);

        transaction_hash_.insert(id, transaction);
        Convert(transaction, trans, node_id == transaction->lhs_node);
        trans_list.emplaceBack(trans);
    }

    return trans_list;
}

SPTransaction SqlOrder::QueryTransaction(int trans_id, QSqlQuery& query)
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
    transaction->transport = query.value("transport").toInt();
    transaction->location = query.value("location").toString().split(SEMICOLON, Qt::SkipEmptyParts);

    return transaction;
}
