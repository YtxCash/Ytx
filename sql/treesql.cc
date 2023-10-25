#include "treesql.h"

#include <QSqlError>
#include <QSqlQuery>

#include "global/nodepool.h"
#include "global/sqlconnection.h"

TreeSql::TreeSql(CString& node, CString& path, CString& trans, Section section)
    : db_ { SqlConnection::Instance().Allocate(section) }
    , node_ { node }
    , path_ { path }
    , trans_ { trans }
{
}

bool TreeSql::Tree(NodeHash& node_hash)
{
    QSqlQuery query(*db_);
    query.setForwardOnly(true);

    auto part_1st = QString("SELECT name, id, code, description, note, node_rule, branch, unit "
                            "FROM %1 "
                            "WHERE removed = 0 ")
                        .arg(node_);

    auto part_2nd = QString("SELECT ancestor, descendant "
                            "FROM %1 "
                            "WHERE distance = 1 ")
                        .arg(path_);

    query.prepare(part_1st);
    if (!query.exec()) {
        qWarning() << "Error in create tree 1 setp " << query.lastError().text();
        return false;
    }

    CreateNodeHash(query, node_hash);
    query.clear();

    query.prepare(part_2nd);
    if (!query.exec()) {
        qWarning() << "Error in create tree 2 setp " << query.lastError().text();
        return false;
    }

    SetRelationship(query, node_hash);

    return true;
}

bool TreeSql::Insert(int parent_id, Node* node)
{
    // root_'s id is -1
    if (!node || node->id == -1)
        return false;

    QSqlQuery query(*db_);

    auto part_1st = QString("INSERT INTO %1 (name, code, description, note, node_rule, branch, unit) "
                            "VALUES (:name, :code, :description, :note, :node_rule, :branch, :unit) ")
                        .arg(node_);

    auto part_2nd = QString("INSERT INTO %1 (ancestor, descendant, distance) "
                            "SELECT ancestor, :node_id, distance + 1 FROM %1 "
                            "WHERE descendant = :parent "
                            "UNION ALL "
                            "SELECT :node_id, :node_id, 0 ")
                        .arg(path_);

    if (!DBTransaction([&]() {
            // 插入节点记录
            query.prepare(part_1st);
            query.bindValue(":name", node->name);
            query.bindValue(":code", node->code);
            query.bindValue(":description", node->description);
            query.bindValue(":note", node->note);
            query.bindValue(":unit", node->unit);
            query.bindValue(":node_rule", node->node_rule);
            query.bindValue(":branch", node->branch);

            if (!query.exec()) {
                qWarning() << "Failed to insert node record: " << query.lastError().text();
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
                qWarning() << "Failed to insert node_path record: " << query.lastError().text();
                return false;
            }

            return true;
        })) {
        qWarning() << "Failed to insert record";
        return false;
    }

    return true;
}

bool TreeSql::Remove(int node_id, bool branch)
{
    QSqlQuery query(*db_);

    auto part_1st = QString("UPDATE %1 "
                            "SET removed = 1 "
                            "WHERE id = :node_id ")
                        .arg(node_);

    auto part_2nd = QString("UPDATE %1 "
                            "SET removed = 1 "
                            "WHERE lhs_node = :node_id OR rhs_node = :node_id ")
                        .arg(trans_);

    if (branch)
        part_2nd = QString("WITH related_nodes AS ( "
                           "SELECT DISTINCT fp1.ancestor, fp2.descendant "
                           "FROM %1 AS fp1 "
                           "INNER JOIN %1 AS fp2 ON fp1.descendant = fp2.ancestor "
                           "WHERE fp2.ancestor = :node_id AND fp2.descendant != :node_id "
                           "AND fp1.ancestor != :node_id) "
                           "UPDATE %1 "
                           "SET distance = distance - 1 "
                           "WHERE ancestor IN (SELECT ancestor FROM related_nodes) AND "
                           "descendant IN (SELECT descendant FROM related_nodes) ")
                       .arg(path_);

    //     auto part_22nd = QString("UPDATE %1 "
    //                                   "SET distance = distance - 1 "
    //                                   "WHERE (descendant IN (SELECT descendant
    //                                   FROM %1 " "WHERE ancestor = :node_id AND
    //                                   ancestor != descendant) AND ancestor IN
    //                                   (SELECT ancestor FROM %1 " "WHERE
    //                                   descendant = :node_id AND ancestor !=
    //                                   descendant)) ")
    //                               .arg(path_);

    auto part_3rd = QString("DELETE FROM %1 WHERE (descendant = :node_id OR ancestor = :node_id) AND distance !=0").arg(path_);

    if (!DBTransaction([&]() {
            query.prepare(part_1st);
            query.bindValue(":node_id", node_id);
            if (!query.exec()) {
                qWarning() << "Failed to remove node record 1st step: " << query.lastError().text();
                return false;
            }

            query.clear();

            query.prepare(part_2nd);
            query.bindValue(":node_id", node_id);
            if (!query.exec()) {
                qWarning() << "Failed to remove node_path record 2nd step: " << query.lastError().text();
                return false;
            }

            query.clear();

            query.prepare(part_3rd);
            query.bindValue(":node_id", node_id);
            if (!query.exec()) {
                qWarning() << "Failed to remove node_path record 3rd step: " << query.lastError().text();
                return false;
            }

            return true;
        })) {
        qWarning() << "Failed to remove node";
        return false;
    }

    return true;
}

bool TreeSql::Update(CString& column, const QVariant& value, int node_id)
{
    QSqlQuery query(*db_);

    auto part = QString("UPDATE %1 "
                        "SET %2 = :value "
                        "WHERE id = :node_id ")
                    .arg(node_, column);

    query.prepare(part);
    query.bindValue(":node_id", node_id);
    query.bindValue(":value", value);

    if (!query.exec()) {
        qWarning() << "Failed to update record: " << query.lastError().text();
        return false;
    }

    return true;
}

bool TreeSql::Drag(int destination_node_id, int node_id)
{
    QSqlQuery query(*db_);

    //    auto part_1st = QString("DELETE FROM %1 WHERE (descendant IN
    //    (SELECT descendant FROM "
    //                                  "%1 WHERE ancestor = :node_id) AND
    //                                  ancestor IN (SELECT ancestor FROM "
    //                                  "%1 WHERE descendant = :node_id AND
    //                                  ancestor != descendant)) ")
    //    .arg(path_);

    auto part_1st = QString("WITH related_nodes AS ( "
                            "SELECT DISTINCT fp1.ancestor, fp2.descendant "
                            "FROM %1 AS fp1 "
                            "INNER JOIN %1 AS fp2 ON fp1.descendant = fp2.ancestor "
                            "WHERE fp2.ancestor = :node_id AND fp1.ancestor != :node_id) "
                            "DELETE FROM %1 "
                            "WHERE ancestor IN (SELECT ancestor FROM related_nodes) AND "
                            "descendant IN (SELECT descendant FROM related_nodes) ")
                        .arg(path_);

    auto part_2nd = QString("INSERT INTO %1 (ancestor, descendant, distance) "
                            "SELECT fp1.ancestor, fp2.descendant, fp1.distance + fp2.distance + 1 "
                            "FROM %1 AS fp1 "
                            "INNER JOIN %1 AS fp2 "
                            "WHERE fp1.descendant = :destination_node_id AND fp2.ancestor = :node_id ")
                        .arg(path_);

    if (!DBTransaction([&]() {
            // 第一个查询
            query.prepare(part_1st);
            query.bindValue(":node_id", node_id);

            if (!query.exec()) {
                qWarning() << "Failed to drag node_path record 1st step: " << query.lastError().text();
                return false;
            }

            query.clear();

            // 第二个查询
            query.prepare(part_2nd);
            query.bindValue(":node_id", node_id);
            query.bindValue(":destination_node_id", destination_node_id);

            if (!query.exec()) {
                qWarning() << "Failed to drag node_path record 2nd step: " << query.lastError().text();
                return false;
            }
            return true;
        })) {
        qWarning() << "Failed to drag node";
        return false;
    }

    return true;
}

bool TreeSql::Usage(int node_id) const
{
    QSqlQuery query(*db_);
    query.setForwardOnly(true);

    auto string = QString("SELECT COUNT(*) FROM %1 "
                          "WHERE (lhs_node = :node_id OR rhs_node = :node_id) AND removed = 0 ")
                      .arg(trans_);
    query.prepare(string);
    query.bindValue(":node_id", node_id);

    if (!query.exec()) {
        qWarning() << "Failed to count times " << query.lastError().text();
        return false;
    }

    query.next();
    int count = query.value(0).toInt();
    if (count >= 1)
        return true;
    else
        return false;
}

void TreeSql::LeafTotal(Node* node)
{
    if (!node || node->id == -1 || node->branch)
        return;

    QSqlQuery query(*db_);
    query.setForwardOnly(true);

    auto part = QString("SELECT lhs_debit AS debit, lhs_credit AS credit, lhs_ratio AS ratio FROM %1 "
                        "WHERE lhs_node = (:node_id) AND removed = 0 "
                        "UNION ALL "
                        "SELECT rhs_debit, rhs_credit, rhs_ratio FROM %1 "
                        "WHERE rhs_node = (:node_id) AND removed = 0 ")
                    .arg(trans_);

    query.prepare(part);
    query.bindValue(":node_id", node->id);
    if (!query.exec())
        qWarning() << "Error in calculate node total setp " << query.lastError().text();

    double foreign_total_debit { 0.0 };
    double foreign_total_credit { 0.0 };
    double base_total_debit { 0.0 };
    double base_total_credit { 0.0 };
    bool node_rule { node->node_rule };

    double ratio { 0.0 };
    double debit { 0.0 };
    double credit { 0.0 };

    while (query.next()) {
        ratio = query.value("ratio").toDouble();
        if (ratio == 0) {
            qWarning() << "Error in calculate node total, ratio can't be zero ";
            continue;
        }

        debit = query.value("debit").toDouble();
        credit = query.value("credit").toDouble();

        base_total_debit += debit * ratio;
        base_total_credit += credit * ratio;

        foreign_total_debit += debit;
        foreign_total_credit += credit;
    }

    node->foreign_total = node_rule ? (foreign_total_credit - foreign_total_debit) : (foreign_total_debit - foreign_total_credit);
    node->base_total = node_rule ? (base_total_credit - base_total_debit) : (base_total_debit - base_total_credit);
}

bool TreeSql::DBTransaction(auto Function)
{
    if (db_->transaction() && Function() && db_->commit()) {
        return true;
    } else {
        db_->rollback();
        qWarning() << "Transaction failed";
        return false;
    }
}

void TreeSql::CreateNodeHash(QSqlQuery& query, NodeHash& node_hash)
{
    int node_id {};
    Node* node {};

    while (query.next()) {
        node = NodePool::Instance().Allocate();
        node_id = query.value("id").toInt();

        node->id = node_id;
        node->name = query.value("name").toString();
        node->description = query.value("description").toString();
        node->note = query.value("note").toString();
        node->node_rule = query.value("node_rule").toBool();
        node->branch = query.value("branch").toBool();
        node->code = query.value("code").toString();
        node->unit = query.value("unit").toInt();

        node_hash.insert(node_id, node);
    }
}

void TreeSql::SetRelationship(QSqlQuery& query, const NodeHash& node_hash)
{
    int ancestor_id {};
    int descendant_id {};
    Node* ancestor {};
    Node* descendant {};

    while (query.next()) {
        ancestor_id = query.value("ancestor").toInt();
        descendant_id = query.value("descendant").toInt();

        ancestor = node_hash.value(ancestor_id);
        descendant = node_hash.value(descendant_id);

        ancestor->children.emplaceBack(descendant);
        descendant->parent = ancestor;
    }
}
