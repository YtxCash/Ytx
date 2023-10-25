#include "editnodeorder.h"

#include <QCompleter>

#include "ui_editnodeorder.h"

EditNodeOrder::EditNodeOrder(Node* node, const SectionRule* section_rule, const NodeHash* stakeholder_node_hash, CStringHash* stakeholder_leaf,
    CStringHash* product_leaf, const Info* info, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::EditNodeOrder)
    , node_ { node }
    , stakeholder_node_hash_ { stakeholder_node_hash }
    , stakeholder_leaf_ { stakeholder_leaf }
    , product_leaf_ { product_leaf }
    , info_ { info }
    , section_rule_ { section_rule }
{
    ui->setupUi(this);
    IniCombo(ui->comboCompany);
    IniCombo(ui->comboEmployee);
    IniDialog();
    IniConnect();
}

EditNodeOrder::~EditNodeOrder() { delete ui; }

void EditNodeOrder::on_chkBoxBranch_toggled(bool checked)
{
    ui->comboCompany->setCurrentIndex(-1);
    ui->comboEmployee->setCurrentIndex(-1);

    ui->label_5->setEnabled(!checked);
    ui->label_6->setEnabled(!checked);
    ui->pBtnPrint->setEnabled(!checked);
    ui->pBtnInsert->setEnabled(!checked);
    ui->dSpinFinalTotal->setEnabled(!checked);
    ui->comboEmployee->setEnabled(!checked);
    ui->dateEdit->setEnabled(!checked);
    ui->chkBoxRefund->setEnabled(!checked);
    ui->dSpinDiscount->setEnabled(!checked);

    ui->label->setText(checked ? tr("Branch") : tr("Company"));
    ui->comboCompany->setInsertPolicy(checked ? QComboBox::InsertAtBottom : QComboBox::NoInsert);
    node_->branch = checked;
}

void EditNodeOrder::IniDialog()
{
    ui->comboCompany->setFocus();

    if (node_->branch)
        on_chkBoxBranch_toggled(true);

    switch (node_->unit) {
    case 0:
        ui->rBtnCash->setChecked(true);
        break;
    case 1:
        ui->rBtnMonthly->setChecked(true);
        break;
    case 2:
        ui->rBtnPending->setChecked(true);
        break;
    default:
        break;
    }

    int mark { info_->section == Section::kSales ? 1 : 2 };
    IniCombo(ui->comboCompany, stakeholder_node_hash_, stakeholder_leaf_, mark);
    IniCombo(ui->comboEmployee, stakeholder_node_hash_, stakeholder_leaf_, 0);

    if (node_->name.isEmpty() && node_->seventh_property == 0) {
        ui->pBtnOkOrder->setEnabled(false);
        ui->pBtnSaveOrder->setEnabled(false);
        return;
    }
}

void EditNodeOrder::IniCombo(QComboBox* combo)
{
    combo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    combo->setFrame(false);
    combo->setEditable(true);
    combo->setInsertPolicy(QComboBox::NoInsert);

    auto completer { new QCompleter(combo->model(), combo) };
    completer->setFilterMode(Qt::MatchContains);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    combo->setCompleter(completer);
}

void EditNodeOrder::IniCombo(QComboBox* combo, const NodeHash* node_hash, CStringHash* path_hash, int mark)
{
    combo->blockSignals(true);
    for (auto it = path_hash->cbegin(); it != path_hash->cend(); ++it)
        if (node_hash->value(it.key())->unit == mark)
            combo->addItem(it.value(), it.key());

    combo->model()->sort(0);
    combo->blockSignals(false);
}

void EditNodeOrder::IniConnect() { }

void EditNodeOrder::Data(Node* node) { }

void EditNodeOrder::SetData() { }

void EditNodeOrder::on_comboCompany_currentTextChanged(const QString& arg1)
{
    if (!node_->branch || arg1.isEmpty())
        return;

    node_->name = arg1;
    ui->pBtnOkOrder->setEnabled(true);
    ui->pBtnSaveOrder->setEnabled(true);
}

void EditNodeOrder::on_comboCompany_currentIndexChanged(int index)
{
    Q_UNUSED(index)

    if (node_->branch)
        return;

    node_->seventh_property = ui->comboCompany->currentData().toInt();
    ui->pBtnOkOrder->setEnabled(true);
    ui->pBtnSaveOrder->setEnabled(true);
}

void EditNodeOrder::on_rBtnCash_clicked() { node_->unit = 0; }

void EditNodeOrder::on_rBtnMonthly_clicked() { node_->unit = 1; }

void EditNodeOrder::on_rBtnPending_clicked() { node_->unit = 2; }

void EditNodeOrder::on_chkBoxRefund_toggled(bool checked) { node_->fifth_property = checked; }

void EditNodeOrder::on_comboEmployee_currentIndexChanged(int index)
{
    Q_UNUSED(index)

    if (node_->branch)
        return;

    node_->second_property = ui->comboEmployee->currentData().toInt();
}
