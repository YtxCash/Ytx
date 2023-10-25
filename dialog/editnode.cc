#include "editnode.h"

#include "ui_editnode.h"

EditNode::EditNode(Node* node, CString* separator, CStringHash* unit_hash, bool node_usage, bool view_opened, CString& parent_path, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::EditNode)
    , node_ { node }
    , separator_ { separator }
    , node_usage_ { node_usage }
    , view_opened_ { view_opened }
    , parent_path_ { parent_path }
{
    ui->setupUi(this);
    IniDialog(unit_hash);
    Data(node);
    IniConnect();
}

EditNode::~EditNode() { delete ui; }

void EditNode::IniDialog(CStringHash* unit_hash)
{
    ui->labNote->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    ui->gridLayout->setContentsMargins(0, 0, 0, 0);
    ui->pBtnOk->setDefault(true);
    ui->plainTextEdit->setTabChangesFocus(true);
    ui->comboUnit->setEditable(true);
    ui->comboUnit->setInsertPolicy(QComboBox::NoInsert);
    ui->lineEditName->setFocus();

    if (!parent_path_.isEmpty())
        parent_path_ += *separator_;

    this->setWindowTitle(parent_path_ + node_->name);

    auto mainwindow_size { qApp->activeWindow()->size() };
    int width { mainwindow_size.width() * 390 / 1920 };
    int height { mainwindow_size.height() * 520 / 1080 };
    this->resize(width, height);

    for (auto it = unit_hash->cbegin(); it != unit_hash->cend(); ++it)
        ui->comboUnit->addItem(it.value(), it.key());

    ui->comboUnit->model()->sort(0);
    ui->lineEditName->setValidator(new QRegularExpressionValidator(QRegularExpression("[\\p{L} ()（）\\d]*"), this));
}

void EditNode::IniConnect()
{
    connect(ui->pBtnCancel, &QPushButton::clicked, this, &QDialog::reject, Qt::UniqueConnection);
    connect(ui->pBtnOk, &QPushButton::clicked, this, &EditNode::RCustomAccept, Qt::UniqueConnection);
    connect(ui->lineEditName, &QLineEdit::textEdited, this, &EditNode::REditName, Qt::UniqueConnection);
}

void EditNode::Data(Node* tmp_node)
{
    int item_index { ui->comboUnit->findData(tmp_node->unit) };
    if (item_index != -1)
        ui->comboUnit->setCurrentIndex(item_index);

    ui->rBtnDDCI->setChecked(tmp_node->node_rule);
    ui->rBtnDICD->setChecked(!tmp_node->node_rule);

    for (const Node* node : tmp_node->parent->children)
        node_name_list_.emplaceBack(node->name);

    if (tmp_node->name.isEmpty())
        return ui->pBtnOk->setEnabled(false);

    ui->lineEditName->setText(tmp_node->name);
    ui->lineEditCode->setText(tmp_node->code);
    ui->lineEditDescription->setText(tmp_node->description);
    ui->plainTextEdit->setPlainText(tmp_node->note);

    ui->chkBoxBranch->setChecked(tmp_node->branch);
    ui->chkBoxBranch->setEnabled(!node_usage_ && tmp_node->children.isEmpty() && !view_opened_);
}

void EditNode::SetData()
{
    node_->name = ui->lineEditName->text().simplified();
    node_->code = ui->lineEditCode->text().simplified();
    node_->description = ui->lineEditDescription->text().simplified();
    node_->unit = ui->comboUnit->currentData().toInt();
    node_->branch = ui->chkBoxBranch->isChecked();
    node_->note = ui->plainTextEdit->toPlainText();

    bool node_rule { ui->rBtnDDCI->isChecked() };
    if (node_->base_total != 0 && node_->node_rule != node_rule) {
        node_->base_total = -node_->base_total;
        node_->foreign_total = -node_->foreign_total;
    }
    node_->node_rule = node_rule;
}

void EditNode::RCustomAccept()
{
    SetData();
    accept();
}

void EditNode::REditName(const QString& arg1)
{
    auto simplified { arg1.simplified() };
    this->setWindowTitle(parent_path_ + simplified);
    ui->pBtnOk->setEnabled(!simplified.isEmpty() && !node_name_list_.contains(simplified));
}
