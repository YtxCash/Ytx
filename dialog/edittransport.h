#ifndef EDITTRANSPORT_H
#define EDITTRANSPORT_H

#include <QComboBox>
#include <QDialog>

#include "component/structclass.h"
#include "table/transportmodel.h"

namespace Ui {
class EditTransport;
}

class EditTransport : public QDialog {
    Q_OBJECT

signals:
    // send to signal station
    void SRetrieveOne(Section section, CSPTrans& trans);

    // send to mainwindow
    void STransportLocation(Section section, int trans_id, int lhs_node_id, int rhs_node_id);
    void SDeleteOne(CStringList& location);

public:
    EditTransport(const Info* info, const Interface* interface, SPTrans& trans, const DataCenter* data_center, QWidget* parent = nullptr);
    ~EditTransport();

private slots:
    void RComboBoxDebitCreditCurrentIndexChanged(int index);
    void RPBtnAddClicked();
    void RComboBoxSectionNameCurrentIndexChanged(int index);

    void on_pBtnRemove_clicked();

    void on_tableView_doubleClicked(const QModelIndex& index);

private:
    void IniDoubleSpinBox();
    void IniConnect();
    void IniSectionName();
    void IniTable();
    void IniDelegate();
    void IniComboBox(QComboBox* combo);
    void ResizeColumn(QHeaderView* header);

    bool HasSelection(QTableView* view);

private:
    Ui::EditTransport* ui;
    SPTrans trans_ {};
    TransportModel* transport_model_ {};

    const Info* info_ {};
    const DataCenter* data_center_ {};
    const Interface* interface_ {};
};

#endif // EDITTRANSPORT_H
