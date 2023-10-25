#ifndef TABLEMODELTASK_H
#define TABLEMODELTASK_H

#include "component/info.h"
#include "component/settings.h"
#include "sql/sql.h"
#include "table/abstracttablemodel.h"

class TableModelTask final : public AbstractTableModel {
    Q_OBJECT

public:
    TableModelTask(const Info* info, const SectionRule* section_rule, QSharedPointer<Sql> sql, int node_id, bool node_rule, QObject* parent = nullptr);
    ~TableModelTask() override;

public slots:
    // receive from table sql
    void RRemoveMulti(const QMultiHash<int, int>& node_trans) override;

    // receive from tree model
    void RNodeRule(int node_id, bool node_rule) override;

    // receive from signal station
    void RAppendOne(CSPCTrans& trans) override;
    void RRetrieveOne(CSPTrans& trans) override;
    void RRemoveOne(int node_id, int trans_id) override;
    void RUpdateBalance(int node_id, int trans_id) override;
    void RMoveMulti(int old_node_id, int new_node_id, const QList<int>& trans_id_list) override;

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
    bool AppendOne(const QModelIndex& parent = QModelIndex()) override;
    bool DeleteOne(int row, const QModelIndex& parent = QModelIndex()) override; // delete trans and related transaction

    int NodeRow(int node_id) const override;
    QModelIndex TransIndex(int trans_id) const override;
    void UpdateState(Check state) override;
    SPTrans GetTrans(const QModelIndex& index) const override { return trans_list_.at(index.row()); }

private:
    // deal with multi transactions
    bool AppendMulti(int node_id, const QList<int>& trans_id_list);
    bool RemoveMulti(const QList<int>& trans_id_list); // just remove trnas, keep related transaction

    bool UpdateDateTime(SPTrans& trans, CString& value);
    bool UpdateDescription(SPTrans& trans, CString& value);
    bool UpdateCode(SPTrans& trans, CString& value);
    bool UpdateRelatedNode(SPTrans& trans, int value);
    bool UpdateState(SPTrans& trans, bool value);

    bool UpdateDebit(SPTrans& trans, double value);
    bool UpdateCredit(SPTrans& trans, double value);
    bool UpdateUnitCost(SPTrans& trans, double value);

    double Balance(bool node_rule, double debit, double credit);
    void AccumulateBalance(const SPTransList& list, int row, bool node_rule);

    void RecycleTrans(SPTransList& list);

private:
    const Info* info_ {};
    const SectionRule* section_rule_ {};
    QSharedPointer<Sql> sql_ {};

    SPTransList trans_list_ {};
    int node_id_ {};
    bool node_rule_ {};
};

#endif // TABLEMODELTASK_H
