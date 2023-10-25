#ifndef SQLORDER_H
#define SQLORDER_H

#include "sql/sql.h"

class SqlOrder final : public Sql {
    Q_OBJECT

public:
    SqlOrder(const Info* info, QObject* parent = nullptr);

    // tree
    bool Tree(NodeHash& node_hash) override;
    bool Insert(int parent_id, Node* node) override;
    void LeafTotal(Node* node) override;

    // table
    SPTransList TransList(int node_id) override;
    bool Insert(CSPTrans& trans) override;

    SPTransList TransList(int node_id, const QList<int>& trans_id_list) override;
    SPTransaction Transaction(int trans_id) override;

private:
    // tree
    void CreateNodeHash(QSqlQuery& query, NodeHash& node_hash) override;

    // table
    SPTransList QueryList(int node_id, QSqlQuery& query) override;
    SPTransaction QueryTransaction(int trans_id, QSqlQuery& query) override;
};

#endif // SQLORDER_H
