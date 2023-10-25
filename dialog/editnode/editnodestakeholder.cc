#include "editnodestakeholder.h"

#include <QTimer>

#include "component/constvalue.h"
#include "ui_editnodestakeholder.h"

EditNodeStakeholder::EditNodeStakeholder(Node* node, const SectionRule* section_rule, CString* separator, const Info* info, QSharedPointer<Sql> sql,
    bool view_opened, CString& parent_path, CStringHash* node_term_hash, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::EditNodeStakeholder)
    , node_ { node }
    , separator_ { separator }
    , node_term_hash_ { node_term_hash }
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

EditNodeStakeholder::~EditNodeStakeholder() { delete ui; }

void EditNodeStakeholder::IniDialog(CStringHash* unit_hash)
{
    ui->lineEditName->setFocus();
    parent_path_ += (parent_path_.isEmpty() ? QString() : *separator_);
    this->setWindowTitle(parent_path_ + node_->name);

    ui->comboMark->blockSignals(true);
    for (auto it = unit_hash->cbegin(); it != unit_hash->cend(); ++it)
        ui->comboMark->addItem(it.value(), it.key());

    ui->comboMark->model()->sort(0);
    ui->comboMark->blockSignals(false);

    ui->comboTerm->blockSignals(true);
    for (auto it = node_term_hash_->cbegin(); it != node_term_hash_->cend(); ++it)
        ui->comboTerm->addItem(it.value(), it.key());

    ui->comboTerm->model()->sort(0);
    ui->comboTerm->blockSignals(false);

    ui->lineEditName->setValidator(new QRegularExpressionValidator(QRegularExpression("[\\p{L} ()（）\\d]*"), this));

    ui->spinBoxPaymentPeriod->setRange(0, IMAX);
    ui->dateEditDeadline->setDisplayFormat(DATE_D);
    ui->dSpinBoxTaxRate->setRange(0.0, DMAX);
    ui->dSpinBoxTaxRate->setDecimals(section_rule_->ratio_decimal);
}

void EditNodeStakeholder::IniConnect() { connect(ui->lineEditName, &QLineEdit::textEdited, this, &EditNodeStakeholder::REditName); }

void EditNodeStakeholder::Data(Node* node)
{
    int mark_index { ui->comboMark->findData(node->unit) };
    ui->comboMark->setCurrentIndex(mark_index);
    DisableComponent(node->unit == 0);

    bool editable { parent_path_.isEmpty() && node->name.isEmpty() };
    ui->comboMark->setEnabled(editable);
    ui->labMark->setEnabled(editable);

    int term_index { ui->comboTerm->findData(node->node_rule) };
    ui->comboTerm->setCurrentIndex(term_index);

    if (node->name.isEmpty()) {
        ui->pBtnOk->setEnabled(false);
        return;
    }

    ui->lineEditName->setText(node->name);
    ui->lineEditCode->setText(node->code);
    ui->lineEditDescription->setText(node->description);
    ui->plainTextEdit->setPlainText(node->note);
    ui->spinBoxPaymentPeriod->setValue(node->first_property);
    ui->dSpinBoxTaxRate->setValue(node->third_property * HUNDRED);
    ui->spinBoxDecimal->setValue(node->second_property);
    ui->dateEditDeadline->setDateTime(QDateTime::fromString(node->date_time, DATE_D));

    ui->chkBoxBranch->setChecked(node->branch);

    int node_id { node->id };
    bool usage { sql_->InternalReferences(node_id) || sql_->ExternalReferences(node_id, Section::kPurchase)
        || sql_->ExternalReferences(node_id, Section::kSales) };
    ui->chkBoxBranch->setEnabled(!usage && node->children.isEmpty() && !view_opened_);
}

void EditNodeStakeholder::DisableComponent(bool disable)
{
    ui->labelTaxRate->setDisabled(disable);
    ui->dSpinBoxTaxRate->setDisabled(disable);
    ui->labelDecimal->setDisabled(disable);
    ui->spinBoxDecimal->setDisabled(disable);
}

void EditNodeStakeholder::REditName(const QString& arg1)
{
    auto simplified { arg1.simplified() };
    this->setWindowTitle(parent_path_ + simplified);
    ui->pBtnOk->setEnabled(!simplified.isEmpty() && !node_name_list_.contains(simplified));
}

void EditNodeStakeholder::on_lineEditName_editingFinished() { node_->name = ui->lineEditName->text(); }

void EditNodeStakeholder::on_lineEditCode_editingFinished() { node_->code = ui->lineEditCode->text(); }

void EditNodeStakeholder::on_lineEditDescription_editingFinished() { node_->description = ui->lineEditDescription->text(); }

void EditNodeStakeholder::on_chkBoxBranch_toggled(bool checked) { node_->branch = checked; }

void EditNodeStakeholder::on_plainTextEdit_textChanged() { node_->note = ui->plainTextEdit->toPlainText(); }

void EditNodeStakeholder::on_comboMark_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    node_->unit = ui->comboMark->currentData().toInt();
    DisableComponent(node_->unit == 0);
}

void EditNodeStakeholder::on_comboTerm_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    node_->node_rule = ui->comboTerm->currentData().toBool();
}

void EditNodeStakeholder::on_spinBoxPaymentPeriod_editingFinished() { node_->first_property = ui->spinBoxPaymentPeriod->value(); }

void EditNodeStakeholder::on_dateEditDeadline_editingFinished() { node_->date_time = ui->dateEditDeadline->dateTime().toString(DATE_D); }

void EditNodeStakeholder::on_dSpinBoxTaxRate_editingFinished() { node_->third_property = ui->dSpinBoxTaxRate->value() / HUNDRED; }

void EditNodeStakeholder::on_spinBoxDecimal_editingFinished() { node_->second_property = ui->spinBoxDecimal->value(); }

void EditNodeStakeholder::changeEvent(QEvent* event)
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
