#include "editnodeproduct.h"

#include <QTimer>

#include "component/constvalue.h"
#include "ui_editnodeproduct.h"

EditNodeProduct::EditNodeProduct(Node* node, const SectionRule* section_rule, CString* separator, const Info* info, QSharedPointer<Sql> sql, bool view_opened,
    CString& parent_path, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::EditNodeProduct)
    , node_ { node }
    , separator_ { separator }
    , section_rule_ { section_rule }
    , sql_ { sql }
    , view_opened_ { view_opened }
    , parent_path_ { parent_path }
{
    ui->setupUi(this);
    IniDialog(&info->unit_hash);
    IniConnect();
    Data(node);
}

EditNodeProduct::~EditNodeProduct() { delete ui; }

void EditNodeProduct::IniDialog(CStringHash* unit_hash)
{
    ui->lineEditName->setFocus();
    parent_path_ += (parent_path_.isEmpty() ? QString() : *separator_);
    this->setWindowTitle(parent_path_ + node_->name);

    ui->comboUnit->blockSignals(true);
    for (auto it = unit_hash->cbegin(); it != unit_hash->cend(); ++it)
        ui->comboUnit->addItem(it.value(), it.key());

    ui->comboUnit->model()->sort(0);
    ui->comboUnit->blockSignals(false);

    ui->lineEditName->setValidator(new QRegularExpressionValidator(QRegularExpression("[\\p{L} ()（）\\d]*"), this));

    ui->dSpinBoxUnitPrice->setRange(0.0, DMAX);
    ui->dSpinBoxCommission->setRange(0.0, DMAX);
    ui->dSpinBoxUnitPrice->setDecimals(section_rule_->ratio_decimal);
    ui->dSpinBoxCommission->setDecimals(section_rule_->ratio_decimal);
}

void EditNodeProduct::IniConnect() { connect(ui->lineEditName, &QLineEdit::textEdited, this, &EditNodeProduct::REditName); }

void EditNodeProduct::Data(Node* node)
{
    int item_index { ui->comboUnit->findData(node->unit) };
    ui->comboUnit->setCurrentIndex(item_index);

    ui->rBtnDDCI->setChecked(node->node_rule);
    ui->rBtnDICD->setChecked(!node->node_rule);

    if (node->name.isEmpty())
        return ui->pBtnOk->setEnabled(false);

    ui->lineEditName->setText(node->name);
    ui->lineEditCode->setText(node->code);
    ui->lineEditDescription->setText(node->description);
    ui->plainTextEdit->setPlainText(node->note);
    ui->dSpinBoxUnitPrice->setValue(node->third_property);
    ui->dSpinBoxCommission->setValue(node->fourth_property);

    ui->chkBoxBranch->setChecked(node->branch);

    int node_id { node->id };
    bool usage { sql_->InternalReferences(node_id) || sql_->ExternalReferences(node_id, Section::kStakeholder)
        || sql_->ExternalReferences(node_id, Section::kPurchase) || sql_->ExternalReferences(node_id, Section::kSales) };

    ui->chkBoxBranch->setEnabled(!usage && node->children.isEmpty() && !view_opened_);
}

void EditNodeProduct::REditName(const QString& arg1)
{
    auto simplified { arg1.simplified() };
    this->setWindowTitle(parent_path_ + simplified);
    ui->pBtnOk->setEnabled(!simplified.isEmpty() && !node_name_list_.contains(simplified));
}

void EditNodeProduct::on_lineEditName_editingFinished() { node_->name = ui->lineEditName->text(); }

void EditNodeProduct::on_lineEditCode_editingFinished() { node_->code = ui->lineEditCode->text(); }

void EditNodeProduct::on_lineEditDescription_editingFinished() { node_->description = ui->lineEditDescription->text(); }

void EditNodeProduct::on_comboUnit_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    node_->unit = ui->comboUnit->currentData().toInt();
}

void EditNodeProduct::on_rBtnDDCI_toggled(bool checked)
{
    if (node_->final_total != 0 && node_->node_rule != checked) {
        node_->final_total = -node_->final_total;
        node_->initial_total = -node_->initial_total;
    }
    node_->node_rule = checked;
}

void EditNodeProduct::on_chkBoxBranch_toggled(bool checked) { node_->branch = checked; }

void EditNodeProduct::on_plainTextEdit_textChanged() { node_->note = ui->plainTextEdit->toPlainText(); }

void EditNodeProduct::on_dSpinBoxUnitPrice_editingFinished() { node_->third_property = ui->dSpinBoxUnitPrice->value(); }

void EditNodeProduct::on_dSpinBoxCommission_editingFinished() { node_->fourth_property = ui->dSpinBoxCommission->value(); }

void EditNodeProduct::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::ActivationChange && this->isActiveWindow()) {
        QTimer::singleShot(100, this, [&, this]() {
            node_name_list_.clear();
            for (const auto& node : node_->parent->children)
                if (node->id != node_->id)
                    node_name_list_.emplaceBack(node->name);

            REditName(ui->lineEditName->text());
        });
    }

    QDialog::changeEvent(event);
}
