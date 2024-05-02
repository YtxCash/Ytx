#ifndef TREESQL_H
#define TREESQL_H

// for finance, network, product, task

#include <QSqlDatabase>

#include "component/info.h"
#include "component/using.h"

class TreeSql {
public:
    TreeSql() = default;
    TreeSql(const Info* info);

    bool Tree(NodeHash& node_hash);
    bool Insert(int parent_id, Node* node);
    bool Remove(int node_id, bool branch = false);
    bool Update(CString& column, const QVariant& value, int node_id);
    bool Drag(int destination_node_id, int node_id);

    bool Usage(int node_id) const;
    void LeafTotal(Node* node);

private:
    bool DBTransaction(auto Function);
    void CreateNodeHash(QSqlQuery& query, NodeHash& node_hash);
    void SetRelationship(QSqlQuery& query, const NodeHash& node_hash);

private:
    QSqlDatabase* db_ {};
    const Info* info_ {};
};

#endif // TREESQL_H
