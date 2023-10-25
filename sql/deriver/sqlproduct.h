#ifndef SQLPRODUCT_H
#define SQLPRODUCT_H

#include "sql/sql.h"

class SqlProduct final : public Sql {
public:
    SqlProduct(const Info* info, QObject* parent = nullptr);

    // tree
    bool Tree(NodeHash& node_hash) override;
    bool Insert(int parent_id, Node* node) override;
    void LeafTotal(Node* node) override;
    bool ExternalReferences(int node_id, Section target) const override;

private:
    // tree
    void CreateNodeHash(QSqlQuery& query, NodeHash& node_hash) override;
};

#endif // SQLPRODUCT_H
