#include "sqltask.h"

#include <QSqlError>
#include <QSqlQuery>

#include "global/nodepool.h"

SqlTask::SqlTask(const Info* info, QObject* parent)
    : Sql(info, parent)
{
}

bool SqlTask::Tree(NodeHash& node_hash)
{
    QSqlQuery query(*db_);
    query.setForwardOnly(true);

    auto part_1st = QString("SELECT name, id, code, description, note, node_rule, branch, unit, initial_total "
                            "FROM %1 "
                            "WHERE removed = 0 ")
                        .arg(info_->node);

    auto part_2nd = QString("SELECT ancestor, descendant "
                            "FROM %1 "
                            "WHERE distance = 1 ")
                        .arg(info_->path);

    query.prepare(part_1st);
    if (!query.exec()) {
        qWarning() << "Error in task create tree 1 setp " << query.lastError().text();
        return false;
    }

    CreateNodeHash(query, node_hash);
    query.clear();

    query.prepare(part_2nd);
    if (!query.exec()) {
        qWarning() << "Error in task create tree 2 setp " << query.lastError().text();
        return false;
    }

    SetRelationship(query, node_hash);

    return true;
}

void SqlTask::LeafTotal(Node* node)
{
    if (!node || node->id == -1 || node->branch)
        return;

    QSqlQuery query(*db_);
    query.setForwardOnly(true);

    auto part = QString("SELECT lhs_debit AS debit, lhs_credit AS credit FROM %1 "
                        "WHERE lhs_node = (:node_id) AND removed = 0 "
                        "UNION ALL "
                        "SELECT rhs_debit, rhs_credit FROM %1 "
                        "WHERE rhs_node = (:node_id) AND removed = 0 ")
                    .arg(info_->transaction);

    query.prepare(part);
    query.bindValue(":node_id", node->id);
    if (!query.exec())
        qWarning() << "Error in calculate node total setp " << query.lastError().text();

    double initial_total_debit { 0.0 };
    double initial_total_credit { 0.0 };
    bool node_rule { node->node_rule };

    double debit { 0.0 };
    double credit { 0.0 };

    while (query.next()) {
        debit = query.value("debit").toDouble();
        credit = query.value("credit").toDouble();

        initial_total_debit += debit;
        initial_total_credit += credit;
    }

    node->initial_total = node_rule ? (initial_total_credit - initial_total_debit) : (initial_total_debit - initial_total_credit);
}

void SqlTask::CreateNodeHash(QSqlQuery& query, NodeHash& node_hash)
{
    int node_id {};
    Node* node {};

    while (query.next()) {
        node = NodePool::Instance().Allocate();
        node_id = query.value("id").toInt();

        node->id = node_id;
        node->name = query.value("name").toString();
        node->code = query.value("code").toString();
        node->description = query.value("description").toString();
        node->note = query.value("note").toString();
        node->node_rule = query.value("node_rule").toBool();
        node->branch = query.value("branch").toBool();
        node->unit = query.value("unit").toInt();
        node->initial_total = query.value("initial_total").toDouble();

        node_hash.insert(node_id, node);
    }
}
