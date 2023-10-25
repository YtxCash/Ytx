#ifndef SQLTASK_H
#define SQLTASK_H

#include "sql/sql.h"

class SqlTask final : public Sql {
public:
    SqlTask(const Info* info, QObject* parent = nullptr);

    // tree
    bool Tree(NodeHash& node_hash) override;
    void LeafTotal(Node* node) override;

private:
    // tree
    void CreateNodeHash(QSqlQuery& query, NodeHash& node_hash) override;
};

#endif // SQLTASK_H
