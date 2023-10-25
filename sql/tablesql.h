#ifndef TABLESQL_H
#define TABLESQL_H

// for finance, network, product, product

#include <QObject>
#include <QSqlDatabase>

#include "component/enumclass.h"
#include "component/using.h"

class TableSql : public QObject {
    Q_OBJECT

signals:
    // send to all table model
    void SRemoveMulti(const QMultiHash<int, int>& node_trans);

    // send to signal station
    void SMoveMulti(Section section, int old_node_id, int new_node_id, const QList<int>& trans_id_list);

    // send to tree model
    void SUpdateMultiTotal(const QList<int>& node_id_list);
    void SRemoveNode(int node_id);

    // send to mainwindow
    void SFreeView(int node_id);

public slots:
    // receive from remove node dialog
    bool RRemoveMulti(int node_id);
    bool RReplaceMulti(int old_node_id, int new_node_id);

public:
    explicit TableSql(QObject* parent = nullptr);
    void SetData(CString& trans, Section section);

    SPTransList TransList(int node_id);
    bool Insert(CSPTrans& trans);
    bool Delete(int trans_id);
    bool Update(int trans_id);
    bool Update(CString& column, const QVariant& value, int trans_id);
    bool Update(CString& column, const QVariant& value, Check state);

    SPTransList TransList(int node_id, const QList<int>& trans_id_list);
    SPTrans Trans();

    inline SPTransactionHash* TransactionHash() { return &transaction_hash_; }

private:
    SPTransList QueryList(int node_id, QSqlQuery& query);
    void Convert(CSPTransaction& transaction, SPTrans& trans, bool left);

    QMultiHash<int, int> RelatedNodeAndTrans(int node_id) const;

private:
    QSqlDatabase* db_ {};
    SPTransactionHash transaction_hash_ {};
    SPTransaction last_insert_transaction_ {};
    QString trans_ {}; // SQL database node transaction table name
    Section section_ {};
};

#endif // TABLESQL_H
