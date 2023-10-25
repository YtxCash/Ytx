#include "edittransport.h"

#include <QCompleter>
#include <QDateTime>

#include "component/constvalue.h"
#include "delegate/datetimer.h"
#include "delegate/transport/transportdoubler.h"
#include "delegate/transport/transportnodeidr.h"
#include "ui_edittransport.h"

EditTransport::EditTransport(const Info* info, const Interface* interface, SPTrans& trans, const DataCenter* data_center, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::EditTransport)
    , trans_ { trans }
    , info_ { info }
    , data_center_ { data_center }
    , interface_ { interface }
{
    ui->setupUi(this);
    IniDialog();
    IniDelegate();
    IniTable();
    IniConnect();
    Data();

    if (!trans->location->isEmpty())
        transport_model_->Query(*trans->location);
}

EditTransport::~EditTransport()
{
    delete transport_model_;
    delete ui;
}

void EditTransport::IniConnect()
{
    connect(ui->comboDebit, &QComboBox::currentIndexChanged, this, &EditTransport::RComboDebitCreditCurrentIndexChanged);
    connect(ui->comboCredit, &QComboBox::currentIndexChanged, this, &EditTransport::RComboDebitCreditCurrentIndexChanged);
    connect(ui->pBtnAdd, &QPushButton::clicked, this, &EditTransport::RPBtnAddClicked);
    connect(ui->comboSection, &QComboBox::currentIndexChanged, this, &EditTransport::RComboSectionCurrentIndexChanged);
}

void EditTransport::IniTable()
{
    auto table_header { &data_center_->info_hash.value(Section::kStakeholder)->table_header };
    auto view { ui->tableView };

    transport_model_ = new TransportModel(table_header, &data_center_->sql_hash, this);
    view->setModel(transport_model_);

    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setAlternatingRowColors(true);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->verticalHeader()->setHidden(true);
    view->setColumnHidden(std::to_underlying(TableColumn::kID), true);
    view->setColumnHidden(std::to_underlying(TableColumn::kDocument), true);
    view->setColumnHidden(std::to_underlying(TableColumn::kState), true);
    view->setColumnHidden(std::to_underlying(TableColumn::kCode), true);

    ResizeColumn(ui->tableView->horizontalHeader());
}

void EditTransport::IniDelegate()
{
    auto view { ui->tableView };

    auto node_id { new TransportNodeIDR(trans_, &data_center_->leaf_path_hash, view) };
    view->setItemDelegateForColumn(std::to_underlying(TableColumn::kLhsNode), node_id);
    view->setItemDelegateForColumn(std::to_underlying(TableColumn::kRhsNode), node_id);

    auto date_time { new DateTimeR(&data_center_->section_rule_hash.value(info_->section)->hide_time, view) };
    view->setItemDelegateForColumn(std::to_underlying(TableColumn::kDateTime), date_time);

    auto value { new TransportDoubleR(trans_, &data_center_->section_rule_hash, true, view) };
    view->setItemDelegateForColumn(std::to_underlying(TableColumn::kLhsDebit), value);
    view->setItemDelegateForColumn(std::to_underlying(TableColumn::kRhsDebit), value);
    view->setItemDelegateForColumn(std::to_underlying(TableColumn::kLhsCredit), value);
    view->setItemDelegateForColumn(std::to_underlying(TableColumn::kRhsCredit), value);

    auto ratio { new TransportDoubleR(trans_, &data_center_->section_rule_hash, false, view) };
    view->setItemDelegateForColumn(std::to_underlying(TableColumn::kLhsRatio), ratio);
    view->setItemDelegateForColumn(std::to_underlying(TableColumn::kRhsRatio), ratio);
}

void EditTransport::IniCombo(QComboBox* combo)
{
    combo->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);
    combo->setFrame(false);
    combo->setEditable(true);
    combo->setInsertPolicy(QComboBox::NoInsert);

    auto completer { new QCompleter(combo->model(), combo) };
    completer->setFilterMode(Qt::MatchContains);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    combo->setCompleter(completer);
}

void EditTransport::Data()
{
    RComboSectionCurrentIndexChanged(0);
    ui->comboDebit->setCurrentIndex(-1);
    ui->comboCredit->setCurrentIndex(-1);
}

void EditTransport::ResizeColumn(QHeaderView* header)
{
    header->setSectionResizeMode(QHeaderView::ResizeToContents);
    header->setSectionResizeMode(std::to_underlying(TableColumn::kDescription), QHeaderView::Stretch);
}

bool EditTransport::HasSelection(QTableView* view)
{
    if (!view)
        return false;
    auto selection_model = view->selectionModel();
    return selection_model && selection_model->hasSelection();
}

void EditTransport::RComboDebitCreditCurrentIndexChanged(int index)
{
    Q_UNUSED(index)

    if (ui->comboCredit->currentIndex() == -1 || ui->comboDebit->currentIndex() == -1)
        return;

    auto debit_id { ui->comboDebit->currentData().toInt() };
    auto credit_id { ui->comboCredit->currentData().toInt() };

    ui->pBtnAdd->setEnabled(debit_id != credit_id);
}

void EditTransport::IniDialog()
{
    ui->pBtnAdd->setDisabled(true);

    bool is_receiver { *trans_->transport == 2 };
    ui->comboDebit->setDisabled(is_receiver);
    ui->comboCredit->setDisabled(is_receiver);
    ui->comboSection->setDisabled(is_receiver);
    ui->dSpinBox->setDisabled(is_receiver);
    ui->pBtnRemove->setDisabled(is_receiver);
    ui->label->setDisabled(is_receiver);
    ui->label_2->setDisabled(is_receiver);
    ui->label_3->setDisabled(is_receiver);
    ui->label_4->setDisabled(is_receiver);

    ui->dSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
    ui->dSpinBox->setDecimals(data_center_->section_rule_hash.value(info_->section)->value_decimal);

    if (is_receiver)
        return;

    ui->dSpinBox->setRange(DMIN, DMAX);
    ui->dSpinBox->setGroupSeparatorShown(true);
    ui->dSpinBox->setValue(*trans_->debit + *trans_->credit);

    IniCombo(ui->comboSection);
    IniCombo(ui->comboCredit);
    IniCombo(ui->comboDebit);
    IniComboSection();
}

void EditTransport::IniComboSection()
{
    auto section { ui->comboSection };
    section->blockSignals(true);

    section->addItem(tr(Finance), std::to_underlying(Section::kFinance));
    section->addItem(tr(Product), std::to_underlying(Section::kProduct));
    section->addItem(tr(Task), std::to_underlying(Section::kTask));

    auto item_index { section->findData(std::to_underlying(info_->section)) };
    if (item_index != -1)
        section->removeItem(item_index);

    section->blockSignals(false);
}

void EditTransport::RPBtnAddClicked()
{
    auto trans_id { *trans_->id };

    Section target_section { ui->comboSection->currentData().toInt() };
    auto section { info_->section };

    auto sql_hash { data_center_->sql_hash };

    auto lhs_node { ui->comboDebit->currentData().toInt() };
    auto rhs_node { ui->comboCredit->currentData().toInt() };
    auto value { ui->dSpinBox->value() };

    auto target_sql { sql_hash.value(target_section) };
    auto target_trans { target_sql->AllocateTrans() };
    *target_trans->transport = 2;

    *target_trans->location = { QString::number(std::to_underlying(section)), QString::number(trans_id) };
    *target_trans->date_time = QDateTime::currentDateTime().toString(DATE_TIME_FST);
    *target_trans->node = lhs_node;
    *target_trans->debit = value;
    *target_trans->related_node = rhs_node;
    *target_trans->related_credit = value;
    target_sql->Insert(target_trans);

    auto target_tree_model { data_center_->tree_model_hash.value(target_section) };
    target_tree_model->RUpdateOneTotal(lhs_node, value, 0, value, 0);
    target_tree_model->RUpdateOneTotal(rhs_node, 0, value, 0, value);

    *trans_->transport = 1;
    QStringList list { QString::number(std::to_underlying(target_section)), QString::number(*target_trans->id) };
    *trans_->location += list;

    auto sql { sql_hash.value(section) };
    sql->Update(info_->transaction, TRANSPORT, 1, trans_id);
    sql->Update(info_->transaction, LOCATION, trans_->location->join(SEMICOLON), trans_id);

    emit SRetrieveOne(target_section, target_trans);
    transport_model_->Add(list);

    ResizeColumn(ui->tableView->horizontalHeader());
}

void EditTransport::RComboSectionCurrentIndexChanged(int index)
{
    Q_UNUSED(index)

    const Section section { ui->comboSection->currentData().toInt() };

    auto leaf_path { data_center_->leaf_path_hash.value(section) };
    auto debit { ui->comboDebit };
    auto credit { ui->comboCredit };

    debit->clear();
    credit->clear();

    for (auto it = leaf_path->cbegin(); it != leaf_path->cend(); ++it) {
        debit->addItem(it.value(), it.key());
        credit->addItem(it.value(), it.key());
    }

    debit->model()->sort(0);
    credit->model()->sort(0);
}

void EditTransport::on_pBtnRemove_clicked()
{
    auto view { ui->tableView };
    if (!HasSelection(view))
        return;

    auto index { view->currentIndex() };
    if (!index.isValid())
        return;

    int row { index.row() };
    transport_model_->Remove(row);

    auto location { trans_->location };
    int location_row { row * 2 };

    emit SDeleteOne(QStringList { location->at(location_row), location->at(location_row + 1) });

    auto sql { data_center_->sql_hash.value(info_->section) };
    auto trans_id { *trans_->id };

    trans_->location->remove(location_row, 2);
    *trans_->transport = trans_->location->isEmpty() ? 0 : 1;

    sql->Update(info_->transaction, LOCATION, *trans_->location, trans_id);
    sql->Update(info_->transaction, TRANSPORT, *trans_->transport, trans_id);
}

void EditTransport::on_tableView_doubleClicked(const QModelIndex& index)
{
    if (!index.isValid())
        return;

    int row { index.row() };
    int location_row { row * 2 };
    auto location { trans_->location };

    Section section { location->at(location_row).toInt() };
    int trans_id { location->at(location_row + 1).toInt() };

    auto lhs_node_id { index.sibling(row, std::to_underlying(TableColumn::kLhsNode)).data().toInt() };
    auto rhs_node_id { index.sibling(row, std::to_underlying(TableColumn::kRhsNode)).data().toInt() };

    emit STransportLocation(section, trans_id, lhs_node_id, rhs_node_id);
}
