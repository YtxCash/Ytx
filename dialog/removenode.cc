#include "removenode.h"

#include <QCompleter>
#include <QMessageBox>

#include "ui_removenode.h"

RemoveNode::RemoveNode(CStringHash* leaf_path, int node_id, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::RemoveNode)
    , node_id_ { node_id }
    , leaf_path_ { leaf_path }
{
    ui->setupUi(this);
    IniDialog();
    IniConnect();
}

RemoveNode::~RemoveNode() { delete ui; }

void RemoveNode::DisableRemove()
{
    ui->rBtnRemoveRecords->setDisabled(true);
    ui->label->setText("This node is going to be removed, but it has external references. Replace them?");
}

void RemoveNode::RCustomAccept()
{
    QMessageBox msg(this);
    msg.setIcon(QMessageBox::Question);
    msg.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);

    auto path { leaf_path_->value(node_id_) };
    QString text {};
    QString informative_text {};

    if (ui->rBtnRemoveRecords->isChecked()) {
        text = tr("Remove Records");
        informative_text = tr("Are you sure you want to remove %1 and its related records?").arg(path);
    }

    if (ui->rBtnReplaceRecords->isChecked()) {
        text = tr("Replace Records");
        informative_text = tr("Are you sure you want to replace %1 with %2 in all related records?").arg(path, ui->comboBox->currentText());
    }

    msg.setText(text);
    msg.setInformativeText(informative_text);

    if (msg.exec() == QMessageBox::Ok) {
        if (ui->rBtnRemoveRecords->isChecked())
            emit SRemoveMulti(node_id_);

        if (ui->rBtnReplaceRecords->isChecked()) {
            int new_node_id { ui->comboBox->currentData().toInt() };
            emit SReplaceMulti(node_id_, new_node_id);
        }

        accept();
    }
}

void RemoveNode::IniDialog()
{
    ui->label->setWordWrap(true);
    ui->rBtnReplaceRecords->setChecked(true);
    ui->pBtnCancel->setDefault(true);
    this->setWindowTitle(tr("Remove %1").arg(leaf_path_->value(node_id_)));

    auto combo { ui->comboBox };
    combo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    combo->setFrame(false);
    combo->setEditable(true);
    combo->setInsertPolicy(QComboBox::NoInsert);

    auto completer { new QCompleter(combo->model(), combo) };
    completer->setFilterMode(Qt::MatchContains);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    combo->setCompleter(completer);

    for (auto it = leaf_path_->cbegin(); it != leaf_path_->cend(); ++it)
        if (it.key() != node_id_)
            ui->comboBox->addItem(it.value(), it.key());

    ui->comboBox->model()->sort(0);
}

void RemoveNode::IniConnect()
{
    connect(ui->pBtnOk, &QPushButton::clicked, this, &RemoveNode::RCustomAccept);
    connect(ui->pBtnCancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui->rBtnReplaceRecords, &QRadioButton::toggled, ui->comboBox, &QComboBox::setEnabled);
}
