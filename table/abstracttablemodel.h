#ifndef ABSTRACTTABLEMODEL_H
#define ABSTRACTTABLEMODEL_H

#include <QAbstractItemModel>

#include "component/enumclass.h"
#include "component/using.h"

class AbstractTableModel : public QAbstractItemModel {
    Q_OBJECT

public:
    AbstractTableModel(QObject* parent = nullptr)
        : QAbstractItemModel(parent)
    {
    }
    virtual ~AbstractTableModel() = default;

signals:
    // send to tree model
    void SUpdateOneTotal(int node_id, double final_debit_diff, double final_credit_diff, double initial_debit_diff, double initial_credit_diff);
    void SUpdateProperty(int node_id, double first, double third, double fourth);
    void SSearch();

    // send to signal station
    void SAppendOne(Section section, CSPCTrans& trans);
    void SRemoveOne(Section section, int node_id, int trans_id);
    void SUpdateBalance(Section section, int node_id, int trans_id);
    void SMoveMulti(Section section, int old_node_id, int new_node_id, const QList<int>& trans_id_list);

    // send to its table view
    void SResizeColumnToContents(int column);

public slots:
    // receive from table sql
    virtual void RRemoveMulti(const QMultiHash<int, int>& node_trans) = 0;

    // receive from tree model
    virtual void RNodeRule(int node_id, bool node_rule) = 0;

    // receive from signal station
    virtual void RAppendOne(CSPCTrans& trans) = 0;
    virtual void RRetrieveOne(CSPTrans& trans) = 0;
    virtual void RRemoveOne(int node_id, int trans_id) = 0;
    virtual void RUpdateBalance(int node_id, int trans_id) = 0;
    virtual void RMoveMulti(int old_node_id, int new_node_id, const QList<int>& trans_id_list) = 0;

public:
    virtual bool AppendOne(const QModelIndex& parent = QModelIndex()) = 0;
    virtual bool DeleteOne(int row, const QModelIndex& parent = QModelIndex()) = 0; // delete trans and related transaction

    virtual int NodeRow(int node_id) const = 0;
    virtual QModelIndex TransIndex(int trans_id) const = 0;
    virtual void UpdateState(Check state) = 0;
    virtual SPTrans GetTrans(const QModelIndex& index) const = 0;
};

#endif // ABSTRACTTABLEMODEL_H
