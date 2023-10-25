#ifndef SQL_H
#define SQL_H

// All virtual functions' default implementations are for finance section.

#include <QObject>
#include <QSqlDatabase>

#include "component/enumclass.h"
#include "component/info.h"
#include "component/using.h"

class Sql : public QObject {
    Q_OBJECT

signals:
    // send to all table model
    void SRemoveMulti(const QMultiHash<int, int>& node_trans);
    // send to signal station
    void SMoveMulti(Section section, int old_node_id, int new_node_id, const QList<int>& trans_id_list);
    void SRemoveMultiReferences(Section target, int node_id, const QList<int>& trans_id_list);
    // send to tree model
    void SUpdateMultiTotal(const QList<int>& node_id_list);
    void SRemoveNode(int node_id);
    // send to mainwindow
    void SFreeView(int node_id);
    // send to sql itsself
    void SReplaceReferences(Section origin, int old_node_id, int new_node_id);

public slots:
    // receive from remove node dialog
    virtual bool RRemoveMulti(int node_id);
    virtual bool RReplaceMulti(int old_node_id, int new_node_id);
    // receive from sql
    virtual bool RReplaceReferences(Section origin, int old_node_id, int new_node_id)
    {
        Q_UNUSED(old_node_id)
        Q_UNUSED(new_node_id)
        Q_UNUSED(origin)
        return false;
    }

public:
    Sql(const Info* info, QObject* parent = nullptr);

    // tree
    virtual bool Tree(NodeHash& node_hash);
    virtual bool Insert(int parent_id, Node* node);
    virtual void LeafTotal(Node* node);
    virtual bool InternalReferences(int node_id) const;
    virtual bool ExternalReferences(int node_id, Section target) const
    {
        Q_UNUSED(node_id)
        Q_UNUSED(target)
        return false;
    };

    bool Remove(int node_id, bool branch = false);
    bool Drag(int destination_node_id, int node_id);

    // table
    virtual SPTransList TransList(int node_id);
    virtual SPTransList TransList(int node_id, const QList<int>& trans_id_list);
    virtual SPTransaction Transaction(int trans_id);
    virtual bool Insert(CSPTrans& trans);

    bool Delete(int trans_id);
    bool Update(int trans_id);
    bool Update(CString& column, CVariant& value, Check state);

    SPTrans AllocateTrans();
    SPTransactionHash* TransactionHash() { return &transaction_hash_; }

    // common
    bool Update(CString& table, CString& column, CVariant& value, int id);

protected:
    // tree
    virtual void CreateNodeHash(QSqlQuery& query, NodeHash& node_hash);

    bool DBTransaction(std::function<bool()> function);
    void SetRelationship(QSqlQuery& query, const NodeHash& node_hash);

    // table
    virtual SPTransList QueryList(int node_id, QSqlQuery& query);
    virtual SPTransaction QueryTransaction(int trans_id, QSqlQuery& query);
    virtual QMultiHash<int, int> RelatedNodeTrans(int node_id) const;

    void Convert(CSPTransaction& transaction, SPTrans& trans, bool left);

protected:
    SPTransactionHash transaction_hash_ {};
    SPTransaction last_transaction_ {};

    QSqlDatabase* db_ {};
    const Info* info_ {};
};

#endif // SQL_H
