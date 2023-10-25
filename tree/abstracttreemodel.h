#ifndef ABSTRACTTREEMODEL_H
#define ABSTRACTTREEMODEL_H

#include <QAbstractItemModel>

#include "component/using.h"
#include "widget/tablewidget.h"

using TableHash = QHash<int, TableWidget*>;
using StringHash = QHash<int, QString>;

class AbstractTreeModel : public QAbstractItemModel {
    Q_OBJECT

public:
    AbstractTreeModel(QObject* parent = nullptr)
        : QAbstractItemModel(parent)
    {
    }
    virtual ~AbstractTreeModel() = default;

signals:
    // send to related table model
    void SNodeRule(int node_id, bool node_rule);

    // send to its view
    void SResizeColumnToContents(int column);

    // send to search dialog
    void SSearch();

    // send to mainwindow
    void SUpdateName(const Node* node);
    void SUpdateDSpinBox();

public slots:
    // receive from table sql
    virtual bool RUpdateMultiTotal(const QList<int>& node_list) = 0;
    // receive from related table model
    virtual void RUpdateOneTotal(int node_id, double final_debit_diff, double final_credit_diff, double initial_debit_diff, double initial_credit_diff) = 0;
    virtual void RUpdateProperty(int node_id, double first, double third, double fourth) = 0;
    // receive from table model
    virtual void RSearch() = 0;
    // receive from table sql
    virtual bool RRemoveNode(int node_id) = 0;

public:
    virtual bool RemoveRow(int row, const QModelIndex& parent = QModelIndex()) = 0;
    virtual bool InsertRow(int row, const QModelIndex& parent, Node* node) = 0;

    virtual CStringHash* LeafPath() const = 0;
    virtual CStringHash* BranchPath() const = 0;
    virtual const NodeHash* GetNodeHash() const = 0;
    virtual const Node* GetNode(int node_id) const = 0;

    virtual Node* GetNode(const QModelIndex& index) const = 0;
    virtual QModelIndex GetIndex(int node_id) const = 0;
    virtual QString Path(int node_id) const = 0;

    virtual void UpdateBranchUnit(Node* node) const = 0;

    virtual void UpdateNode(const Node* tmp_node) = 0;
    virtual void UpdateSeparator(CString& separator) = 0;
};

#endif // ABSTRACTTREEMODEL_H
