#include "sqlstakeholder.h"

#include <QSqlError>
#include <QSqlQuery>

#include "component/constvalue.h"
#include "global/nodepool.h"
#include "global/transactionpool.h"
#include "global/transpool.h"

SqlStakeholder::SqlStakeholder(const Info* info, QObject* parent)
    : Sql(info, parent)
{
}

bool SqlStakeholder::Tree(NodeHash& node_hash)
{
    QSqlQuery query(*db_);
    query.setForwardOnly(true);

    auto part_1st = QString("SELECT name, id, code, payment_period, employee, tax_rate, deadline, description, note, term, branch, mark "
                            "FROM %1 "
                            "WHERE removed = 0 ")
                        .arg(info_->node);

    auto part_2nd = QString("SELECT ancestor, descendant "
                            "FROM %1 "
                            "WHERE distance = 1 ")
                        .arg(info_->path);

    query.prepare(part_1st);
    if (!query.exec()) {
        qWarning() << "sqlstakeholder: bool SqlStakeholder::Tree(NodeHash& node_hash) 1st" << query.lastError().text();
        return false;
    }

    CreateNodeHash(query, node_hash);
    query.clear();

    query.prepare(part_2nd);
    if (!query.exec()) {
        qWarning() << "sqlstakeholder: bool SqlStakeholder::Tree(NodeHash& node_hash) 2nd" << query.lastError().text();
        return false;
    }

    SetRelationship(query, node_hash);

    return true;
}

bool SqlStakeholder::Insert(int parent_id, Node* node)
{
    // root_'s id is -1
    if (!node || node->id == -1)
        return false;

    QSqlQuery query(*db_);

    auto part_1st = QString("INSERT INTO %1 (name, code, payment_period, employee, tax_rate, deadline, description, note, term, branch, mark) "
                            "VALUES (:name, :code, :payment_period, :employee, :tax_rate, :deadline, :description, :note, :term, :branch, :mark) ")
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
            query.bindValue(":code", node->code);
            query.bindValue(":payment_period", node->first_property);
            query.bindValue(":employee", node->second_property);
            query.bindValue(":tax_rate", node->third_property);
            query.bindValue(":deadline", node->date_time);
            query.bindValue(":description", node->description);
            query.bindValue(":note", node->note);
            query.bindValue(":term", node->node_rule);
            query.bindValue(":branch", node->branch);
            query.bindValue(":mark", node->unit);

            if (!query.exec()) {
                qWarning() << "sqlstakeholder: bool SqlStakeholder::Insert(int parent_id, Node* node) 1st" << query.lastError().text();
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
                qWarning() << "sqlstakeholder: bool SqlStakeholder::Insert(int parent_id, Node* node) 2nd" << query.lastError().text();
                return false;
            }

            return true;
        })) {
        qWarning() << "sqlstakeholder: bool SqlStakeholder::Insert(int parent_id, Node* node) end";
        return false;
    }

    return true;
}

bool SqlStakeholder::InternalReferences(int node_id) const
{
    QSqlQuery query(*db_);
    query.setForwardOnly(true);

    auto string = QString("SELECT COUNT(*) FROM %1 "
                          "WHERE lhs_node = :node_id AND removed = 0 ")
                      .arg(info_->transaction);
    query.prepare(string);
    query.bindValue(":node_id", node_id);

    if (!query.exec()) {
        qWarning() << "sqlstakeholder: bool SqlStakeholder::InternalReferences(int node_id) const" << query.lastError().text();
        return false;
    }

    query.next();
    return query.value(0).toInt() >= 1;
}

bool SqlStakeholder::ExternalReferences(int node_id, Section target) const
{
    QSqlQuery query(*db_);
    query.setForwardOnly(true);

    QString transaction {};

    switch (target) {
    case Section::kSales:
        transaction = "sales_transaction";
        break;
    case Section::kPurchase:
        transaction = "purchase_transaction";
        break;
    default:
        break;
    }

    auto string = QString("SELECT COUNT(*) FROM %1 "
                          "WHERE lhs_node = :node_id AND removed = 0 ")
                      .arg(transaction);
    query.prepare(string);
    query.bindValue(":node_id", node_id);

    if (!query.exec()) {
        qWarning() << "sqlstakeholder: bool SqlStakeholder::ExternalReferences(int node_id, Section target) const" << query.lastError().text();
        return false;
    }

    query.next();
    return query.value(0).toInt() >= 1;
}

bool SqlStakeholder::RRemoveMulti(int node_id)
{
    auto list { TransID(node_id) };

    // begin deal with database
    QSqlQuery query(*db_);

    auto part = QString("UPDATE %1 "
                        "SET removed = 1 "
                        "WHERE lhs_node = :node_id ")
                    .arg(info_->transaction);

    query.prepare(part);
    query.bindValue(":node_id", node_id);
    if (!query.exec()) {
        qWarning() << "sqlstakeholder: bool SqlStakeholder::RRemoveMulti(int node_id)" << query.lastError().text();
        return false;
    }
    // end deal with database

    emit SFreeView(node_id);
    emit SRemoveNode(node_id);

    // begin deal with transaction hash
    for (int trans_id : list)
        TransactionPool::Instance().Recycle(transaction_hash_.take(trans_id));
    // end deal with transaction hash

    return true;
}

bool SqlStakeholder::RReplaceMulti(int old_node_id, int new_node_id)
{
    auto node_trans { RelatedNodeTrans(old_node_id) };

    // begin deal with transaction hash
    for (auto& transaction : std::as_const(transaction_hash_))
        if (transaction->lhs_node == old_node_id)
            transaction->lhs_node = new_node_id;
    // end deal with transaction hash

    // begin deal with database
    QSqlQuery query(*db_);
    auto part = QString("UPDATE %1 "
                        "SET lhs_node = :new_node_id "
                        "WHERE lhs_node = :old_node_id ")
                    .arg(info_->transaction);

    query.prepare(part);
    query.bindValue(":new_node_id", new_node_id);
    query.bindValue(":old_node_id", old_node_id);
    if (!query.exec()) {
        qWarning() << "sqlstakeholder: bool SqlStakeholder::RReplaceMulti(int old_node_id, int new_node_id)" << query.lastError().text();
        return false;
    }
    // end deal with database

    emit SFreeView(old_node_id);
    emit SRemoveNode(old_node_id);
    emit SMoveMulti(info_->section, old_node_id, new_node_id, node_trans.values());

    return true;
}

bool SqlStakeholder::RReplaceReferences(Section origin, int old_node_id, int new_node_id)
{
    Q_UNUSED(origin)

    QSqlQuery query(*db_);
    auto part = QString("UPDATE %1 "
                        "SET rhs_node = :new_node_id "
                        "WHERE rhs_node = :old_node_id ")
                    .arg(info_->transaction);

    query.prepare(part);
    query.bindValue(":old_node_id", old_node_id);
    query.bindValue(":new_node_id", new_node_id);
    if (!query.exec()) {
        qWarning() << "sqlstakeholder: " << query.lastError().text();
        return false;
    }

    for (auto& transaction : std::as_const(transaction_hash_))
        if (transaction->rhs_node == old_node_id)
            transaction->rhs_node = new_node_id;

    return true;
}

SPTransList SqlStakeholder::TransList(int lhs_node_id)
{
    QSqlQuery query(*db_);
    query.setForwardOnly(true);

    auto part = QString("SELECT id, date_time, code, lhs_node, unit_price, commission, description, document, rhs_node "
                        "FROM %1 "
                        "WHERE lhs_node = :lhs_node_id AND removed = 0 ")
                    .arg(info_->transaction);

    query.prepare(part);
    query.bindValue(":lhs_node_id", lhs_node_id);

    if (!query.exec()) {
        qWarning() << "sqlstakeholder: SPTransList SqlStakeholder::TransList(int lhs_node_id)" << query.lastError().text();
        return SPTransList();
    }

    return QueryList(lhs_node_id, query);
}

bool SqlStakeholder::Insert(CSPTrans& trans)
{
    QSqlQuery query(*db_);
    auto part = QString("INSERT INTO %1 "
                        "(date_time, code, lhs_node, unit_price, commission, description, document, rhs_node) "
                        "VALUES "
                        "(:date_time, :code, :lhs_node, :unit_price, :commission, :description, :document, :rhs_node) ")
                    .arg(info_->transaction);

    query.prepare(part);
    query.bindValue(":date_time", *trans->date_time);
    query.bindValue(":code", *trans->code);
    query.bindValue(":lhs_node", *trans->node);
    query.bindValue(":unit_price", *trans->ratio);
    query.bindValue(":commission", *trans->related_debit);
    query.bindValue(":description", *trans->description);
    query.bindValue(":document", trans->document->join(SEMICOLON));
    query.bindValue(":rhs_node", *trans->related_node);

    if (!query.exec()) {
        qWarning() << "sqlstakeholder: bool SqlStakeholder::Insert(CSPTrans& trans)" << query.lastError().text();
        return false;
    }

    *trans->id = query.lastInsertId().toInt();
    transaction_hash_.insert(*trans->id, last_transaction_);
    return true;
}

SPTransList SqlStakeholder::TransList(int node_id, const QList<int>& trans_id_list)
{
    QSqlQuery query(*db_);
    query.setForwardOnly(true);

    QStringList list {};

    for (const auto& id : trans_id_list)
        list.append(QString::number(id));

    auto part = QString("SELECT id, date_time, code, lhs_node, unit_price, commission, description, document, rhs_node "
                        "FROM %1 "
                        "WHERE id IN (%2) AND lhs_node = :node_id AND removed = 0 ")
                    .arg(info_->transaction, list.join(", "));

    query.prepare(part);
    query.bindValue(":node_id", node_id);

    if (!query.exec()) {
        qWarning() << "sqlstakeholder: SPTransList SqlStakeholder::TransList(int node_id, const QList<int>& trans_id_list)" << query.lastError().text();
        return SPTransList();
    }

    return QueryList(node_id, query);
}

SPTransaction SqlStakeholder::Transaction(int trans_id)
{
    QSqlQuery query(*db_);
    query.setForwardOnly(true);

    auto part = QString("SELECT id, date_time, code, lhs_node, unit_price, commission, description,  document, rhs_node "
                        "FROM %1 "
                        "WHERE id = :trans_id AND removed = 0 ")
                    .arg(info_->transaction);

    query.prepare(part);
    query.bindValue(":trans_id", trans_id);

    if (!query.exec()) {
        qWarning() << "sqlstakeholder: SPTransaction SqlStakeholder::Transaction(int trans_id)" << query.lastError().text();
        return SPTransaction();
    }

    return QueryTransaction(trans_id, query);
}

void SqlStakeholder::CreateNodeHash(QSqlQuery& query, NodeHash& node_hash)
{
    int node_id {};
    Node* node {};

    while (query.next()) {
        node = NodePool::Instance().Allocate();
        node_id = query.value("id").toInt();

        node->id = node_id;
        node->name = query.value("name").toString();
        node->description = query.value("description").toString();
        node->third_property = query.value("tax_rate").toDouble();
        node->second_property = query.value("employee").toInt();
        node->first_property = query.value("payment_period").toInt();
        node->date_time = query.value("deadline").toString();
        node->note = query.value("note").toString();
        node->node_rule = query.value("term").toBool();
        node->branch = query.value("branch").toBool();
        node->code = query.value("code").toString();
        node->unit = query.value("mark").toInt();

        node_hash.insert(node_id, node);
    }
}

SPTransList SqlStakeholder::QueryList(int node_id, QSqlQuery& query)
{
    Q_UNUSED(node_id)

    SPTrans trans {};
    SPTransList trans_list {};
    SPTransaction transaction {};
    int id {};

    while (query.next()) {
        id = query.value("id").toInt();
        trans = TransPool::Instance().Allocate();

        if (transaction_hash_.contains(id)) {
            transaction = transaction_hash_.value(id);
            Convert(transaction, trans, true);
            trans_list.emplaceBack(trans);
            continue;
        }

        transaction = TransactionPool::Instance().Allocate();
        transaction->id = id;

        transaction->lhs_node = query.value("lhs_node").toInt();
        transaction->rhs_node = query.value("rhs_node").toInt();
        transaction->lhs_ratio = query.value("unit_price").toDouble();
        transaction->rhs_debit = query.value("commission").toDouble();
        transaction->code = query.value("code").toString();
        transaction->description = query.value("description").toString();
        transaction->document = query.value("document").toString().split(SEMICOLON, Qt::SkipEmptyParts);
        transaction->date_time = query.value("date_time").toString();

        transaction_hash_.insert(id, transaction);
        Convert(transaction, trans, true);
        trans_list.emplaceBack(trans);
    }

    return trans_list;
}

SPTransaction SqlStakeholder::QueryTransaction(int trans_id, QSqlQuery& query)
{
    if (transaction_hash_.contains(trans_id))
        return transaction_hash_.value(trans_id);

    SPTransaction transaction { TransactionPool::Instance().Allocate() };
    query.next();

    transaction->id = trans_id;
    transaction->lhs_node = query.value("lhs_node").toInt();
    transaction->lhs_ratio = query.value("unit_price").toDouble();
    transaction->rhs_node = query.value("rhs_node").toInt();
    transaction->rhs_debit = query.value("commission").toInt();
    transaction->code = query.value("code").toString();
    transaction->description = query.value("description").toString();
    transaction->document = query.value("document").toString().split(SEMICOLON, Qt::SkipEmptyParts);
    transaction->date_time = query.value("date_time").toString();

    return transaction;
}

QList<int> SqlStakeholder::TransID(int node_id, bool lhs)
{
    QSqlQuery query(*db_);
    query.setForwardOnly(true);

    QList<int> list {};
    auto part = QString("SELECT id FROM %1 "
                        "WHERE lhs_node = :node_id AND removed = 0 ")
                    .arg(info_->transaction);

    if (!lhs)
        part = QString("SELECT id FROM %1 "
                       "WHERE rhs_node = :node_id AND removed = 0 ")
                   .arg(info_->transaction);

    query.prepare(part);
    query.bindValue(":node_id", node_id);

    if (!query.exec()) {
        qWarning() << "sqlstakeholder: QList<int> SqlStakeholder::TransID(int node_id, bool lhs)" << query.lastError().text();
        return QList<int>();
    }

    while (query.next())
        list.emplaceBack(query.value("id").toInt());

    return list;
}
