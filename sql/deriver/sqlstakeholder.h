#ifndef SQLSTAKEHOLDER_H
#define SQLSTAKEHOLDER_H

#include "sql/sql.h"

class SqlStakeholder final : public Sql {
    Q_OBJECT

public slots:
    // receive from remove node dialog
    bool RRemoveMulti(int node_id) override;
    bool RReplaceMulti(int old_node_id, int new_node_id) override;
    // receive from product sql
    bool RReplaceReferences(Section origin, int old_node_id, int new_node_id) override;

public:
    SqlStakeholder(const Info* info, QObject* parent = nullptr);

    // tree
    bool Tree(NodeHash& node_hash) override;
    bool Insert(int parent_id, Node* node) override;
    bool InternalReferences(int node_id) const override;
    bool ExternalReferences(int node_id, Section target) const override;

    // table
    SPTransList TransList(int lhs_node_id) override;
    bool Insert(CSPTrans& trans) override;

    SPTransList TransList(int node_id, const QList<int>& trans_id_list) override;
    SPTransaction Transaction(int trans_id) override;

private:
    // tree
    void CreateNodeHash(QSqlQuery& query, NodeHash& node_hash) override;

    // table
    SPTransList QueryList(int node_id, QSqlQuery& query) override;
    SPTransaction QueryTransaction(int trans_id, QSqlQuery& query) override;

    QList<int> TransID(int node_id, bool lhs = true);
};

#endif // SQLSTAKEHOLDER_H
