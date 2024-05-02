#include "search.h"

#include <QHeaderView>

#include "component/constvalue.h"
#include "component/enumclass.h"
#include "delegate/datetimer.h"
#include "delegate/search/snodeidr.h"
#include "delegate/state.h"
#include "delegate/table/tabledbclick.h"
#include "delegate/table/tablenodeid.h"
#include "delegate/table/tablevalue.h"
#include "delegate/tree/treecombo.h"
#include "delegate/tree/treevaluer.h"
#include "ui_search.h"

Search::Search(const Info* info, const Interface* interface, const TreeModel* tree_model, SearchSql* sql, const SectionRule* section_rule,
    const CStringHash* node_rule, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::Search)
    , sql_ { sql }
    , node_rule_ { node_rule }
    , section_rule_ { section_rule }
    , tree_model_ { tree_model }
    , info_ { info }
    , interface_ { interface }
{
    ui->setupUi(this);

    IniDialog();

    search_tree_model_ = new SearchTreeModel(info_, tree_model_, section_rule_, sql, this);
    search_table_model_ = new SearchTableModel(info, sql, this);

    IniTree(ui->treeView, search_tree_model_);
    IniTable(ui->tableView, search_table_model_);
    IniConnect();

    IniView(ui->treeView);
    HideColumn(ui->treeView, info->section);
    IniView(ui->tableView);

    ResizeTreeColumn(ui->treeView->horizontalHeader());
    ResizeTableColumn(ui->tableView->horizontalHeader());
}

Search::~Search() { delete ui; }

void Search::IniDialog()
{
    ui->rBtnNode->setChecked(true);
    ui->pBtnCancel->setAutoDefault(false);
    ui->page->setContentsMargins(0, 0, 0, 0);
    ui->page2->setContentsMargins(0, 0, 0, 0);
    this->setWindowTitle(tr("Search"));

    auto mainwindow_size { qApp->activeWindow()->size() };
    int width { mainwindow_size.width() * 800 / 1920 };
    int height { mainwindow_size.height() * 600 / 1080 };
    this->resize(width, height);
}

void Search::IniConnect()
{
    connect(ui->pBtnCancel, &QPushButton::clicked, this, &Search::close, Qt::UniqueConnection);
    connect(ui->lineEdit, &QLineEdit::returnPressed, this, &Search::RSearch, Qt::UniqueConnection);
    connect(ui->treeView, &QTableView::doubleClicked, this, &Search::RDoubleClicked, Qt::UniqueConnection);
    connect(ui->tableView, &QTableView::doubleClicked, this, &Search::RDoubleClicked, Qt::UniqueConnection);
}

void Search::IniTree(QTableView* view, SearchTreeModel* model)
{
    view->setModel(model);

    auto unit { new TreeCombo(&info_->unit_hash, view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kUnit), unit);

    auto node_rule { new TreeCombo(node_rule_, view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kNodeRule), node_rule);

    auto total { new TreeValueR(&section_rule_->value_decimal, &info_->unit_symbol_hash, view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kTotal), total);

    auto branch { new State(view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kBranch), branch);

    auto leaf_path { tree_model_->LeafPath() };
    auto branch_path { tree_model_->BranchPath() };
    auto name { new SNodeIDR(leaf_path, branch_path, view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kName), name);

    view->setColumnHidden(std::to_underlying(TreeColumn::kID), true);
    view->setColumnHidden(std::to_underlying(TreeColumn::kPlaceholder), true);
}

void Search::IniTable(QTableView* view, SearchTableModel* model)
{
    view->setModel(model);

    auto value { new TableValue(&section_rule_->value_decimal, DMIN, DMAX, view) };
    view->setItemDelegateForColumn(std::to_underlying(TableColumn::kLhsDebit), value);
    view->setItemDelegateForColumn(std::to_underlying(TableColumn::kRhsDebit), value);
    view->setItemDelegateForColumn(std::to_underlying(TableColumn::kLhsCredit), value);
    view->setItemDelegateForColumn(std::to_underlying(TableColumn::kRhsCredit), value);

    auto ratio { new TableValue(&section_rule_->ratio_decimal, DMIN, DMAX, view) };
    view->setItemDelegateForColumn(std::to_underlying(TableColumn::kLhsRatio), ratio);
    view->setItemDelegateForColumn(std::to_underlying(TableColumn::kRhsRatio), ratio);

    auto leaf_path { tree_model_->LeafPath() };
    auto node_name { new TableNodeID(leaf_path, 0, view) };
    view->setItemDelegateForColumn(std::to_underlying(TableColumn::kRhsNode), node_name);
    view->setItemDelegateForColumn(std::to_underlying(TableColumn::kLhsNode), node_name);

    auto state { new State(view) };
    view->setItemDelegateForColumn(std::to_underlying(TableColumn::kState), state);

    auto document { new TableDbClick(view) };
    view->setItemDelegateForColumn(std::to_underlying(TableColumn::kDocument), document);

    auto date_time { new DateTimeR(&interface_->date_format, &section_rule_->hide_time, view) };
    view->setItemDelegateForColumn(std::to_underlying(TableColumn::kDateTime), date_time);

    view->setColumnHidden(std::to_underlying(TableColumn::kID), true);
}

void Search::IniView(QTableView* view)
{
    view->setSortingEnabled(true);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setAlternatingRowColors(true);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->verticalHeader()->setHidden(true);
}

void Search::HideColumn(QTableView* view, Section section)
{
    switch (section) {
    case Section::kFinance:
        view->setColumnHidden(std::to_underlying(TreeColumn::kRatio), true);
        view->setColumnHidden(std::to_underlying(TreeColumn::kExtension), true);
        view->setColumnHidden(std::to_underlying(TreeColumn::kDeadline), true);
        break;
    case Section::kTask:
    case Section::kProduct:
        view->setColumnHidden(std::to_underlying(TreeColumn::kExtension), true);
        view->setColumnHidden(std::to_underlying(TreeColumn::kDeadline), true);
        break;
    case Section::kNetwork:
        view->setColumnHidden(std::to_underlying(TreeColumn::kTotal), true);
        view->setColumnHidden(std::to_underlying(TreeColumn::kNodeRule), true);
        view->setColumnHidden(std::to_underlying(TreeColumn::kUnit), true);
        break;
    default:
        break;
    }
}

void Search::ResizeTreeColumn(QHeaderView* header)
{
    header->setSectionResizeMode(QHeaderView::ResizeToContents);
    header->setSectionResizeMode(std::to_underlying(TreeColumn::kDescription), QHeaderView::Stretch);
}

void Search::ResizeTableColumn(QHeaderView* header)
{
    header->setSectionResizeMode(QHeaderView::ResizeToContents);
    header->setSectionResizeMode(std::to_underlying(TableColumn::kDescription), QHeaderView::Stretch);
}

void Search::RSearch()
{
    CString kText { ui->lineEdit->text() };

    if (ui->rBtnNode->isChecked()) {
        search_tree_model_->Query(kText);
        ResizeTreeColumn(ui->treeView->horizontalHeader());
    }

    if (ui->rBtnTransaction->isChecked()) {
        search_table_model_->Query(kText);
        ResizeTableColumn(ui->tableView->horizontalHeader());
    }
}

void Search::RDoubleClicked(const QModelIndex& index)
{
    if (ui->rBtnNode->isChecked()) {
        int node_id { index.sibling(index.row(), std::to_underlying(TreeColumn::kID)).data().toInt() };
        emit STreeLocation(node_id);
    }

    if (ui->rBtnTransaction->isChecked()) {
        int lhs_node_id { index.sibling(index.row(), std::to_underlying(TableColumn::kLhsNode)).data().toInt() };
        int rhs_node_id { index.sibling(index.row(), std::to_underlying(TableColumn::kRhsNode)).data().toInt() };
        int trans_id { index.sibling(index.row(), std::to_underlying(TableColumn::kID)).data().toInt() };
        emit STableLocation(trans_id, lhs_node_id, rhs_node_id);
    }
}

void Search::on_rBtnNode_toggled(bool checked)
{
    if (checked)
        ui->stackedWidget->setCurrentIndex(0);
}

void Search::on_rBtnTransaction_toggled(bool checked)
{
    if (checked)
        ui->stackedWidget->setCurrentIndex(1);
}
