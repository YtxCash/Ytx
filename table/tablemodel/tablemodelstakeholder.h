#ifndef TABLEMODELSTAKEHOLDER_H
#define TABLEMODELSTAKEHOLDER_H

#include "component/info.h"
#include "component/settings.h"
#include "sql/sql.h"
#include "table/abstracttablemodel.h"

class TableModelStakeholder final : public AbstractTableModel {
    Q_OBJECT

public:
    TableModelStakeholder(const Info* info, const SectionRule* section_rule, QSharedPointer<Sql> sql, int node_id, int mark, QObject* parent = nullptr);
    ~TableModelStakeholder() override;

public slots:
    // receive from table sql
    void RRemoveMulti(const QMultiHash<int, int>&) override { }
    // receive from tree model
    void RNodeRule(int, bool) override { }
    // receive from signal station
    void RAppendOne(CSPCTrans&) override { }
    void RRetrieveOne(CSPTrans&) override { }
    void RRemoveOne(int, int) override { }
    void RUpdateBalance(int, int) override { }

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
    void UpdateState(Check state) override { Q_UNUSED(state) };
    SPTrans GetTrans(const QModelIndex& index) const override { return trans_list_.at(index.row()); }

private:
    // deal with multi transactions
    bool AppendMulti(int node_id, const QList<int>& trans_id_list);
    bool RemoveMulti(const QList<int>& trans_id_list); // just remove trnas, keep related transaction

    bool UpdateDateTime(SPTrans& trans, CString& value);
    bool UpdateDescription(SPTrans& trans, CString& value);
    bool UpdateCode(SPTrans& trans, CString& value);
    bool UpdateRelatedNode(SPTrans& trans, int value);
    bool UpdateUnitPrice(SPTrans& trans, double value);
    bool UpdateCommission(SPTrans& trans, double value);

    void RecycleTrans(SPTransList& list);

private:
    const Info* info_ {};
    const SectionRule* section_rule_ {};
    QSharedPointer<Sql> sql_ {};

    SPTransList trans_list_ {};
    int node_id_ {};
    int mark_ {};
};

#endif // TABLEMODELSTAKEHOLDER_H
