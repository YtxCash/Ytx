#include "editnodefinance.h"

#include <QTimer>

#include "ui_editnodefinance.h"

EditNodeFinance::EditNodeFinance(Node* node, CString* separator, const Info* info, bool node_usage, bool view_opened, CString& parent_path, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::EditNodeFinance)
    , node_ { node }
    , separator_ { separator }
    , node_usage_ { node_usage }
    , view_opened_ { view_opened }
    , parent_path_ { parent_path }
{
    ui->setupUi(this);
    IniDialog(&info->unit_hash);
    IniConnect();
    Data(node);

    if (info->section == Section::kFinance)
        ui->labUnit->setText(tr("Currency"));
}

EditNodeFinance::~EditNodeFinance() { delete ui; }

void EditNodeFinance::IniDialog(CStringHash* unit_hash)
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
}

void EditNodeFinance::IniConnect() { connect(ui->lineEditName, &QLineEdit::textEdited, this, &EditNodeFinance::REditName); }

void EditNodeFinance::Data(Node* node)
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

    ui->chkBoxBranch->setChecked(node->branch);
    ui->chkBoxBranch->setEnabled(!node_usage_ && node->children.isEmpty() && !view_opened_);
}

void EditNodeFinance::REditName(const QString& arg1)
{
    auto simplified { arg1.simplified() };
    this->setWindowTitle(parent_path_ + simplified);
    ui->pBtnOk->setEnabled(!simplified.isEmpty() && !node_name_list_.contains(simplified));
}

void EditNodeFinance::on_lineEditName_editingFinished() { node_->name = ui->lineEditName->text(); }

void EditNodeFinance::on_lineEditCode_editingFinished() { node_->code = ui->lineEditCode->text(); }

void EditNodeFinance::on_lineEditDescription_editingFinished() { node_->description = ui->lineEditDescription->text(); }

void EditNodeFinance::on_comboUnit_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    node_->unit = ui->comboUnit->currentData().toInt();
}

void EditNodeFinance::on_rBtnDDCI_toggled(bool checked)
{
    if (node_->final_total != 0 && node_->node_rule != checked) {
        node_->final_total = -node_->final_total;
        node_->initial_total = -node_->initial_total;
    }
    node_->node_rule = checked;
}

void EditNodeFinance::on_chkBoxBranch_toggled(bool checked) { node_->branch = checked; }

void EditNodeFinance::on_plainTextEdit_textChanged() { node_->note = ui->plainTextEdit->toPlainText(); }

void EditNodeFinance::changeEvent(QEvent* event)
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
