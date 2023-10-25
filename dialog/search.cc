#include "search.h"

#include <QHeaderView>

#include "component/enumclass.h"
#include "delegate/search/nodename.h"
#include "delegate/state.h"
#include "delegate/table/document.h"
#include "delegate/table/nodeid.h"
#include "delegate/table/tablevalue.h"
#include "delegate/tree/combohash.h"
#include "delegate/tree/treevalue.h"
#include "ui_search.h"

Search::Search(const TreeInfo* tree_info, const TreeModel* tree_model, const TableInfo* table_info, SearchSql* sql, const SectionRule* section_rule,
    CStringHash* unit_hash, CStringHash* node_rule_hash, CStringHash* unit_symbol_hash, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::Search)
    , sql_ { sql }
    , unit_hash_ { unit_hash }
    , node_rule_hash_ { node_rule_hash }
    , section_rule_ { section_rule }
    , tree_model_ { tree_model }
    , tree_info_ { tree_info }
    , unit_symbol_hash_ { unit_symbol_hash }
{
    ui->setupUi(this);

    IniDialog();

    node_model_ = new SearchNodeModel(tree_info_, tree_model_, section_rule_, sql, this);
    trans_model_ = new SearchTransModel(table_info, sql, this);

    IniNode(ui->nodeView, node_model_);
    IniTrans(ui->transView, trans_model_);
    IniConnect();

    IniView(ui->nodeView);
    IniView(ui->transView);
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
    connect(ui->nodeView, &QTableView::doubleClicked, this, &Search::RDoubleClicked, Qt::UniqueConnection);
    connect(ui->transView, &QTableView::doubleClicked, this, &Search::RDoubleClicked, Qt::UniqueConnection);
}

void Search::IniNode(QTableView* view, SearchNodeModel* model)
{
    view->setModel(model);

    auto unit { new ComboHash(unit_hash_, view) };
    view->setItemDelegateForColumn(static_cast<int>(NodeColumn::kUnit), unit);

    auto node_rule { new ComboHash(node_rule_hash_, view) };
    view->setItemDelegateForColumn(static_cast<int>(NodeColumn::kNodeRule), node_rule);

    auto total { new TreeValue(&section_rule_->value_decimal, unit_symbol_hash_, view) };
    view->setItemDelegateForColumn(static_cast<int>(NodeColumn::kTotal), total);

    auto branch { new State(view) };
    view->setItemDelegateForColumn(static_cast<int>(NodeColumn::kBranch), branch);

    auto leaf_path { tree_model_->LeafPath() };
    auto branch_path { tree_model_->BranchPath() };
    auto name { new NodeName(leaf_path, branch_path, view) };
    view->setItemDelegateForColumn(static_cast<int>(NodeColumn::kName), name);

    view->setColumnHidden(static_cast<int>(NodeColumn::kID), true);
    view->setColumnHidden(static_cast<int>(NodeColumn::kPlaceholder), true);

    auto h_header { view->horizontalHeader() };
    auto count { model->columnCount() };

    for (int column = 0; column != count; ++column) {
        NodeColumn x { column };

        switch (x) {
        case NodeColumn::kDescription:
            h_header->setSectionResizeMode(column, QHeaderView::Stretch);
            break;
        default:
            h_header->setSectionResizeMode(column, QHeaderView::ResizeToContents);
            break;
        }
    }
}

void Search::IniTrans(QTableView* view, SearchTransModel* model)
{
    view->setModel(model);

    auto value { new TableValue(&section_rule_->value_decimal, view) };
    view->setItemDelegateForColumn(static_cast<int>(SearchTransactionColumn::kLhsDebit), value);
    view->setItemDelegateForColumn(static_cast<int>(SearchTransactionColumn::kRhsDebit), value);
    view->setItemDelegateForColumn(static_cast<int>(SearchTransactionColumn::kLhsCredit), value);
    view->setItemDelegateForColumn(static_cast<int>(SearchTransactionColumn::kRhsCredit), value);

    auto ratio_decimal { new TableValue(&section_rule_->ratio_decimal, view) };
    view->setItemDelegateForColumn(static_cast<int>(SearchTransactionColumn::kLhsRatio), ratio_decimal);
    view->setItemDelegateForColumn(static_cast<int>(SearchTransactionColumn::kRhsRatio), ratio_decimal);

    auto leaf_path { tree_model_->LeafPath() };
    auto node_name { new NodeID(leaf_path, 0, view) };
    view->setItemDelegateForColumn(static_cast<int>(SearchTransactionColumn::kRhsNode), node_name);
    view->setItemDelegateForColumn(static_cast<int>(SearchTransactionColumn::kLhsNode), node_name);

    auto state { new State(view) };
    view->setItemDelegateForColumn(static_cast<int>(SearchTransactionColumn::kState), state);

    auto document { new Document(view) };
    view->setItemDelegateForColumn(static_cast<int>(SearchTransactionColumn::kDocument), document);

    view->setColumnHidden(static_cast<int>(SearchTransactionColumn::kID), true);

    auto h_header { view->horizontalHeader() };
    auto count { model->columnCount() };

    for (int column = 0; column != count; ++column) {
        SearchTransactionColumn x { column };

        switch (x) {
        case SearchTransactionColumn::kDescription:
            h_header->setSectionResizeMode(column, QHeaderView::Stretch);
            break;
        default:
            h_header->setSectionResizeMode(column, QHeaderView::ResizeToContents);
            break;
        }
    }
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

void Search::RSearch()
{
    CString kText { ui->lineEdit->text() };

    if (ui->rBtnNode->isChecked()) {
        node_model_->Query(kText);
        ui->nodeView->resizeColumnsToContents();
    }

    if (ui->rBtnTransaction->isChecked()) {
        trans_model_->Query(kText);
        ui->transView->resizeColumnsToContents();
    }
}

void Search::RDoubleClicked(const QModelIndex& index)
{
    if (ui->rBtnNode->isChecked()) {
        int node_id { index.sibling(index.row(), static_cast<int>(NodeColumn::kID)).data().toInt() };
        emit SSearchedNode(node_id);
    }

    if (ui->rBtnTransaction->isChecked()) {
        int lhs_node_id { index.sibling(index.row(), static_cast<int>(SearchTransactionColumn::kLhsNode)).data().toInt() };
        int rhs_node_id { index.sibling(index.row(), static_cast<int>(SearchTransactionColumn::kRhsNode)).data().toInt() };
        int trans_id { index.sibling(index.row(), static_cast<int>(SearchTransactionColumn::kID)).data().toInt() };
        emit SSearchedTrans(trans_id, lhs_node_id, rhs_node_id);
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
