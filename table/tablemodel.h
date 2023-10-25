#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QAbstractItemModel>

#include "component/enumclass.h"
#include "component/settings.h"
#include "component/using.h"
#include "sql/tablesql.h"
#include "tableinfo.h"

class TableModel : public QAbstractItemModel {
    Q_OBJECT

public:
    TableModel(const TableInfo* info, TableSql* sql, const Interface* interface, int node_id, bool node_rule, QObject* parent = nullptr);
    ~TableModel();

signals:
    // send to tree model
    void SUpdateOneTotal(int node_id, double base_debit_diff, double base_credit_diff, double foreign_debit_diff, double foreign_credit_diff);
    void SSearch();

    // send to signal station
    void SAppendOne(Section section, CSPCTrans& trans);
    void SDeleteOne(Section section, int node_id, int trans_id);
    void SUpdateBalance(Section section, int node_id, int trans_id);
    void SMoveMulti(Section section, int old_node_id, int new_node_id, const QList<int>& trans_id_list);

    // send to its table view
    void SResizeColumnToContents(int column);

public slots:
    // receive from table sql
    void RRemoveMulti(const QMultiHash<int, int>& node_trans);

    // receive from tree model
    void RNodeRule(int node_id, bool node_rule);

    // receive from signal station
    void RAppendOne(CSPCTrans& trans);
    void RDeleteOne(int node_id, int trans_id);
    void RUpdateBalance(int node_id, int trans_id);
    void RMoveMulti(int old_node_id, int new_node_id, const QList<int>& trans_id_list);

public:
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void sort(int column, Qt::SortOrder order) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

public:
    bool AppendOne(const QModelIndex& parent = QModelIndex());
    bool DeleteOne(int row, const QModelIndex& parent = QModelIndex()); // delete trans and associated transaction

public:
    int NodeRow(int node_id) const;
    QModelIndex TransIndex(int trans_id) const;
    void UpdateState(Check state);
    inline SPTrans GetTrans(const QModelIndex& index) const { return trans_list_.at(index.row()); }

private:
    // deal with multi transactions
    bool AppendMulti(int node_id, const QList<int>& trans_id_list);
    bool RemoveMulti(const QList<int>& trans_id_list); // just remove trnas, keep associated transaction

    bool UpdateDateTime(SPTrans& trans, CString& value);
    bool UpdateDescription(SPTrans& trans, CString& value);
    bool UpdateCode(SPTrans& trans, CString& value);
    bool UpdateRelatedNode(SPTrans& trans, int value);
    bool UpdateState(SPTrans& trans, bool value);
    bool UpdateDebit(SPTrans& trans, double value);
    bool UpdateCredit(SPTrans& trans, double value);
    bool UpdateRatio(SPTrans& trans, double value);

    double Balance(bool node_rule, double debit, double credit);
    void AccumulateBalance(const SPTransList& list, int row, bool node_rule);

    void RecycleTrans(SPTransList& list);

private:
    SPTransList trans_list_ {};
    TableSql* sql_ {};

    const TableInfo* info_ {};
    const Interface* interface_ {};

    int node_id_ {};
    bool node_rule_ {};
};

#endif // TABLEMODEL_H
