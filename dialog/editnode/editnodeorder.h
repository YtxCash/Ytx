#ifndef EDITORDER_H
#define EDITORDER_H

#include <QComboBox>
#include <QDialog>

#include "component/info.h"
#include "component/settings.h"
#include "component/using.h"

namespace Ui {
class EditNodeOrder;
}

class EditNodeOrder final : public QDialog {
    Q_OBJECT

public:
    EditNodeOrder(Node* node, const SectionRule* section_rule, const NodeHash* stakeholder_node_hash, CStringHash* stakeholder_leaf, CStringHash* product_leaf,
        const Info* info, QWidget* parent = nullptr);
    ~EditNodeOrder();

private slots:
    void on_chkBoxBranch_toggled(bool checked);

    void on_comboCompany_currentTextChanged(const QString& arg1);

    void on_comboCompany_currentIndexChanged(int index);

    void on_rBtnCash_clicked();

    void on_rBtnMonthly_clicked();

    void on_rBtnPending_clicked();

    void on_chkBoxRefund_toggled(bool checked);

    void on_comboEmployee_currentIndexChanged(int index);

private:
    void IniDialog();
    void IniCombo(QComboBox* combo);
    void IniCombo(QComboBox* combo, const NodeHash* node_hash, CStringHash* path_hash, int mark);
    void IniConnect();
    void Data(Node* node);
    void SetData();

private:
    Ui::EditNodeOrder* ui;

    Node* node_ {};
    const NodeHash* stakeholder_node_hash_ {};
    CStringHash* stakeholder_leaf_ {};
    CStringHash* product_leaf_ {};
    const Info* info_ {};
    const SectionRule* section_rule_ {};
};

#endif // EDITORDER_H
