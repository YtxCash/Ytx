#include "mainwindow.h"

#include <QDragEnterEvent>
#include <QFileDialog>
#include <QHeaderView>
#include <QLockFile>
#include <QMessageBox>
#include <QMimeData>
#include <QProcess>
#include <QResource>
#include <QScrollBar>

#include "component/constvalue.h"
#include "component/enumclass.h"
#include "delegate/checkstate.h"
#include "delegate/datetimed.h"
#include "delegate/line.h"
#include "delegate/table/tablecombo.h"
#include "delegate/table/tabledbclick.h"
#include "delegate/table/tabledoublespin.h"
#include "delegate/tree/deadline.h"
#include "delegate/tree/treecombo.h"
#include "delegate/tree/treedoublespin.h"
#include "delegate/tree/treedoublespinpercent.h"
#include "delegate/tree/treedoublespinr.h"
#include "delegate/tree/treeplaintext.h"
#include "delegate/tree/treespin.h"
#include "dialog/about.h"
#include "dialog/editdocument.h"
#include "dialog/editnode/editnodefinance.h"
#include "dialog/editnode/editnodeorder.h"
#include "dialog/editnode/editnodeproduct.h"
#include "dialog/editnode/editnodestakeholder.h"
#include "dialog/edittransport.h"
#include "dialog/preferences.h"
#include "dialog/removenode.h"
#include "dialog/search.h"
#include "global/nodepool.h"
#include "global/signalstation.h"
#include "global/sqlconnection.h"
#include "sql/deriver/sqlorder.h"
#include "sql/deriver/sqlproduct.h"
#include "sql/deriver/sqlstakeholder.h"
#include "sql/deriver/sqltask.h"
#include "table/tablemodel/tablemodelfinance.h"
#include "table/tablemodel/tablemodelproduct.h"
#include "table/tablemodel/tablemodelstakeholder.h"
#include "table/tablemodel/tablemodeltask.h"
#include "tree/treemodel/treemodelfinance.h"
#include "tree/treemodel/treemodelorder.h"
#include "tree/treemodel/treemodelproduct.h"
#include "tree/treemodel/treemodelstakeholder.h"
#include "tree/treemodel/treemodeltask.h"
#include "ui_mainwindow.h"
#include "widget/tablewidget.h"
#include "widget/treewidget/treewidget.h"
#include "widget/treewidget/treewidgetorder.h"

MainWindow::MainWindow(CString& dir_path, QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , dir_path_ { dir_path }
{
    ResourceFile();
    SharedInterface(dir_path);

    ui->setupUi(this);
    SetTabWidget();
    SetConnect();
    SetHash();
    SetHeader();
    SetAction();

    qApp->setWindowIcon(QIcon(":/logo/logo/logo.png"));
    this->setAcceptDrops(true);

    RestoreState(ui->splitter, shared_interface_, WINDOW, SPLITTER_STATE);
    RestoreState(this, shared_interface_, WINDOW, MAINWINDOW_STATE);
    RestoreGeometry(this, shared_interface_, WINDOW, MAINWINDOW_GEOMETRY);

    Recent();

#ifdef Q_OS_WIN
    ui->actionRemove->setShortcut(Qt::Key_Delete);
#elif defined(Q_OS_MACOS)
    ui->actionRemove->setShortcut(Qt::Key_Backspace);
#endif
}

MainWindow::~MainWindow()
{
    SaveState(ui->splitter, shared_interface_, WINDOW, SPLITTER_STATE);
    SaveState(this, shared_interface_, WINDOW, MAINWINDOW_STATE);
    SaveGeometry(this, shared_interface_, WINDOW, MAINWINDOW_GEOMETRY);
    shared_interface_->setValue(START_SECTION, std::to_underlying(start_));

    if (finance_tree_.widget) {
        SaveTableWidget(finance_table_, finance_data_.info.node, VIEW);
        SaveState(finance_tree_.widget->Header(), exclusive_interface_, finance_data_.info.node, HEADER_STATE);

        finance_dialog_.clear();
    }

    if (product_tree_.widget) {
        SaveTableWidget(product_table_, product_data_.info.node, VIEW);
        SaveState(product_tree_.widget->Header(), exclusive_interface_, product_data_.info.node, HEADER_STATE);

        product_dialog_.clear();
    }

    if (stakeholder_tree_.widget) {
        SaveTableWidget(stakeholder_table_, stakeholder_data_.info.node, VIEW);
        SaveState(stakeholder_tree_.widget->Header(), exclusive_interface_, stakeholder_data_.info.node, HEADER_STATE);

        stakeholder_dialog_.clear();
    }

    if (task_tree_.widget) {
        SaveTableWidget(task_table_, task_data_.info.node, VIEW);
        SaveState(task_tree_.widget->Header(), exclusive_interface_, task_data_.info.node, HEADER_STATE);

        task_dialog_.clear();
    }

    if (sales_tree_.widget) {
        SaveTableWidget(sales_table_, sales_data_.info.node, VIEW);
        SaveState(sales_tree_.widget->Header(), exclusive_interface_, sales_data_.info.node, HEADER_STATE);

        sales_dialog_.clear();
    }

    if (purchase_tree_.widget) {
        SaveTableWidget(purchase_table_, purchase_data_.info.node, VIEW);
        SaveState(purchase_tree_.widget->Header(), exclusive_interface_, purchase_data_.info.node, HEADER_STATE);

        purchase_dialog_.clear();
    }

    delete ui;
}

void MainWindow::ROpenFile(CString& file_path)
{
    if (file_path.isEmpty() || file_path == SqlConnection::Instance().DatabaseName())
        return;

    if (SqlConnection::Instance().DatabaseEnable()) {
        QProcess::startDetached(qApp->applicationFilePath(), QStringList() << file_path);
        return;
    }

    if (SqlConnection::Instance().SetDatabaseName(file_path)) {
        QFileInfo file_info(file_path);
        auto complete_base_name { file_info.completeBaseName() };

        this->setWindowTitle(complete_base_name);
        ExclusiveInterface(dir_path_, complete_base_name);
        LockFile(file_info.absolutePath(), complete_base_name);

        sql_ = MainwindowSql(start_);
        SetFinanceData();
        SetTaskData();
        SetProductData();
        SetStakeholderData();
        SetSalesData();
        SetPurchaseData();

        CreateSection(finance_tree_, tr(Finance), &finance_data_, &finance_table_, &finance_rule_);
        CreateSection(stakeholder_tree_, tr(Stakeholder), &stakeholder_data_, &stakeholder_table_, &stakeholder_rule_);
        CreateSection(product_tree_, tr(Product), &product_data_, &product_table_, &product_rule_);
        CreateSection(task_tree_, tr(Task), &task_data_, &task_table_, &task_rule_);
        CreateSection(sales_tree_, tr(Sales), &sales_data_, &sales_table_, &sales_rule_);
        CreateSection(purchase_tree_, tr(Purchase), &purchase_data_, &purchase_table_, &purchase_rule_);

        SetDataCenter(data_center);

        switch (start_) {
        case Section::kFinance:
            on_rBtnFinance_clicked();
            break;
        case Section::kStakeholder:
            on_rBtnStakeholder_clicked();
            break;
        case Section::kProduct:
            on_rBtnProduct_clicked();
            break;
        case Section::kTask:
            on_rBtnTask_clicked();
            break;
        case Section::kSales:
            on_rBtnSales_clicked();
            break;
        case Section::kPurchase:
            on_rBtnPurchase_clicked();
            break;
        default:
            break;
        }

        QString path { QDir::toNativeSeparators(file_path) };

        if (!recent_list_.contains(path)) {
            auto menu { ui->menuRecent };
            auto action { new QAction(path, menu) };

            if (menu->isEmpty()) {
                menu->addAction(action);
                SetClearMenuAction();
            } else
                ui->menuRecent->insertAction(ui->actionSeparator, action);

            recent_list_.emplaceBack(path);
            UpdateRecent();
        }
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        auto suffix { QFileInfo(event->mimeData()->urls().at(0).fileName()).suffix().toLower() };
        if (suffix == YTX)
            return event->acceptProposedAction();
    }

    event->ignore();
}

void MainWindow::dropEvent(QDropEvent* event) { ROpenFile(event->mimeData()->urls().at(0).toLocalFile()); }

void MainWindow::RTreeViewDoubleClicked(const QModelIndex& index)
{
    if (index.column() != 0)
        return;

    if (bool branch { index.siblingAtColumn(std::to_underlying(TreeColumn::kBranch)).data().toBool() })
        return;

    auto node { section_tree_->model->GetNode(index) };
    if (node->id == -1)
        return;

    if (!section_table_->contains(node->id))
        CreateTable(section_data_, section_tree_->model, section_rule_, section_table_, node);

    SwitchTab(node->id);
}

void MainWindow::SwitchTab(int node_id, int trans_id)
{
    auto widget { section_table_->value(node_id, nullptr) };
    if (!widget)
        return;

    ui->tabWidget->setCurrentWidget(widget);
    widget->activateWindow();

    if (trans_id == 0)
        return;

    auto view { widget->View() };
    auto index { GetTableModel(view)->TransIndex(trans_id) };
    view->setCurrentIndex(index);
    view->scrollTo(index.sibling(index.row(), std::to_underlying(PartTableColumn::kDateTime)), QAbstractItemView::PositionAtCenter);
}

bool MainWindow::LockFile(CString& absolute_path, CString& complete_base_name)
{
    auto lock_file_path { absolute_path + SLASH + complete_base_name + SFX_LOCK };

    static QLockFile lock_file { lock_file_path };
    if (!lock_file.tryLock(100))
        return false;

    return true;
}

void MainWindow::CreateTable(SectionData* data, AbstractTreeModel* tree_model, const SectionRule* section_rule, TableHash* table_hash, const Node* node)
{
    auto node_id { node->id };
    auto node_name { node->name };
    auto section { data->info.section };

    auto widget { new TableWidget(this) };
    auto view { widget->View() };
    AbstractTableModel* model {};

    switch (section) {
    case Section::kFinance:
        model = new TableModelFinance(&data->info, section_rule, data->sql, node_id, node->node_rule, this);
        break;
    case Section::kProduct:
        model = new TableModelProduct(&data->info, section_rule, data->sql, node_id, node->node_rule, this);
        break;
    case Section::kTask:
        model = new TableModelTask(&data->info, section_rule, data->sql, node_id, node->node_rule, this);
        break;
    case Section::kStakeholder:
        model = new TableModelStakeholder(&data->info, section_rule, data->sql, node_id, node->unit, this);
        break;
    default:
        break;
    }

    widget->SetModel(model);

    int tab_index { ui->tabWidget->addTab(widget, node_name) };
    auto tab_bar { ui->tabWidget->tabBar() };

    tab_bar->setTabData(tab_index, QVariant::fromValue(Tab { section, node_id }));
    tab_bar->setTabToolTip(tab_index, tree_model->Path(node_id));

    SetView(view);
    SetConnect(view, model, tree_model, data);

    switch (section) {
    case Section::kFinance:
    case Section::kProduct:
    case Section::kTask:
        CreateDelegate(view, tree_model, section_rule, node_id);
        break;
    case Section::kStakeholder:
        DelegateStakeholder(view, section_rule);
        break;
    default:
        break;
    }

    table_hash->insert(node_id, widget);
    SignalStation::Instance().RegisterModel(section, node_id, model);
}

void MainWindow::SetConnect(const QTableView* view, const AbstractTableModel* table, const AbstractTreeModel* tree, const SectionData* data)
{
    connect(table, &AbstractTableModel::SResizeColumnToContents, view, &QTableView::resizeColumnToContents);

    connect(table, &AbstractTableModel::SRemoveOne, &SignalStation::Instance(), &SignalStation::RRemoveOne);
    connect(table, &AbstractTableModel::SAppendOne, &SignalStation::Instance(), &SignalStation::RAppendOne);
    connect(table, &AbstractTableModel::SUpdateBalance, &SignalStation::Instance(), &SignalStation::RUpdateBalance);
    connect(table, &AbstractTableModel::SMoveMulti, &SignalStation::Instance(), &SignalStation::RMoveMulti);

    connect(table, &AbstractTableModel::SUpdateOneTotal, tree, &AbstractTreeModel::RUpdateOneTotal);
    connect(table, &AbstractTableModel::SSearch, tree, &AbstractTreeModel::RSearch);
    connect(table, &AbstractTableModel::SUpdateProperty, tree, &AbstractTreeModel::RUpdateProperty);

    connect(tree, &AbstractTreeModel::SNodeRule, table, &AbstractTableModel::RNodeRule);

    connect(data->sql.data(), &Sql::SRemoveMulti, table, &AbstractTableModel::RRemoveMulti);

    connect(data->sql.data(), &Sql::SMoveMulti, &SignalStation::Instance(), &SignalStation::RMoveMulti);
}

void MainWindow::CreateDelegate(QTableView* view, const AbstractTreeModel* tree_model, const SectionRule* section_rule, int node_id)
{
    auto node { new TableCombo(tree_model->LeafPath(), node_id, view) };
    view->setItemDelegateForColumn(std::to_underlying(PartTableColumn::kRelatedNode), node);

    auto value { new TableDoubleSpin(&section_rule->value_decimal, DMIN, DMAX, view) };
    view->setItemDelegateForColumn(std::to_underlying(PartTableColumn::kDebit), value);
    view->setItemDelegateForColumn(std::to_underlying(PartTableColumn::kCredit), value);
    view->setItemDelegateForColumn(std::to_underlying(PartTableColumn::kRemainder), value);

    auto ratio { new TableDoubleSpin(&section_rule->ratio_decimal, DMIN, DMAX, view) };
    view->setItemDelegateForColumn(std::to_underlying(PartTableColumn::kRatio), ratio);

    auto date_time { new DateTimeD(&interface_.date_format, &section_rule->hide_time, true, view) };
    view->setItemDelegateForColumn(std::to_underlying(PartTableColumn::kDateTime), date_time);

    auto line { new Line(view) };
    view->setItemDelegateForColumn(std::to_underlying(PartTableColumn::kDescription), line);
    view->setItemDelegateForColumn(std::to_underlying(PartTableColumn::kCode), line);

    auto state { new CheckState(false, view) };
    view->setItemDelegateForColumn(std::to_underlying(PartTableColumn::kState), state);

    auto document { new TableDbClick(view) };
    view->setItemDelegateForColumn(std::to_underlying(PartTableColumn::kDocument), document);
    connect(document, &TableDbClick::SEdit, this, &MainWindow::REditDocument);

    auto transport { new TableDbClick(view) };
    view->setItemDelegateForColumn(std::to_underlying(PartTableColumn::kTransport), transport);
    connect(transport, &TableDbClick::SEdit, this, &MainWindow::REditTransport);
}

void MainWindow::DelegateStakeholder(QTableView* view, const SectionRule* section_rule)
{
    auto node { new TableCombo(product_tree_.model->LeafPath(), 0, view) };
    view->setItemDelegateForColumn(std::to_underlying(PartTableColumn::kRelatedNode), node);

    auto unit_price { new TableDoubleSpin(&section_rule->ratio_decimal, DMIN, DMAX, view) };
    view->setItemDelegateForColumn(std::to_underlying(PartTableColumn::kRatio), unit_price);

    auto date_time { new DateTimeD(&interface_.date_format, &section_rule->hide_time, true, view) };
    view->setItemDelegateForColumn(std::to_underlying(PartTableColumn::kDateTime), date_time);

    auto line { new Line(view) };
    view->setItemDelegateForColumn(std::to_underlying(PartTableColumn::kDescription), line);
    view->setItemDelegateForColumn(std::to_underlying(PartTableColumn::kCode), line);

    auto document { new TableDbClick(view) };
    view->setItemDelegateForColumn(std::to_underlying(PartTableColumn::kDocument), document);
    connect(document, &TableDbClick::SEdit, this, &MainWindow::REditDocument);
}

void MainWindow::CreateSection(Tree& tree, CString& name, SectionData* data, TableHash* table_hash, const SectionRule* section_rule)
{
    const auto* info { &data->info };
    auto tab_widget { ui->tabWidget };

    auto widget { tree.widget };
    auto view { widget->View() };
    auto model { tree.model };

    CreateDelegate(view, info, section_rule);
    SetConnect(view, widget, model, data->sql.data());

    tab_widget->tabBar()->setTabData(tab_widget->addTab(widget, name), QVariant::fromValue(Tab { info->section, 0 }));

    RestoreState(view->header(), exclusive_interface_, info->node, HEADER_STATE);
    RestoreTableWidget(data, model, section_rule, table_hash, VIEW);

    SetView(view);
    // HideNodeColumn(view, info->section);
}

void MainWindow::CreateDelegate(QTreeView* view, const Info* info, const SectionRule* section_rule)
{
    switch (info->section) {
    case Section::kFinance:
    case Section::kTask:
        DelegateFinance(view, info, section_rule);
        break;
    case Section::kStakeholder:
        DelegateStakeholder(view, info, section_rule);
        break;
    case Section::kProduct:
        DelegateProduct(view, info, section_rule);
        break;
    case Section::kSales:
    case Section::kPurchase:
        DelegateOrder(view, info, section_rule);
        break;
    default:
        break;
    }
}

void MainWindow::DelegateFinance(QTreeView* view, const Info* info, const SectionRule* section_rule)
{
    auto line { new Line(view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kDescription), line);
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kCode), line);

    auto plain_text { new TreePlainText(view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kNote), plain_text);

    auto total { new TreeDoubleSpinR(&section_rule->value_decimal, &info->unit_symbol_hash, view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kInitialTotal), total);

    auto unit { new TreeCombo(&info->unit_hash, view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kUnit), unit);

    auto node_rule { new TreeCombo(&node_rule_hash_, view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kNodeRule), node_rule);

    auto branch { new CheckState(true, view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kBranch), branch);
}

void MainWindow::DelegateProduct(QTreeView* view, const Info* info, const SectionRule* section_rule)
{
    auto line { new Line(view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kDescription), line);
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kCode), line);

    auto plain_text { new TreePlainText(view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kNote), plain_text);

    auto total { new TreeDoubleSpinR(&section_rule->value_decimal, &info->unit_symbol_hash, view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kInitialTotal), total);

    auto unit_price { new TreeDoubleSpin(&section_rule->ratio_decimal, DMIN, DMAX, view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kThird), unit_price);

    auto unit { new TreeCombo(&info->unit_hash, view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kUnit), unit);

    auto node_rule { new TreeCombo(&node_rule_hash_, view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kNodeRule), node_rule);

    auto branch { new CheckState(true, view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kBranch), branch);
}

void MainWindow::DelegateStakeholder(QTreeView* view, const Info* info, const SectionRule* section_rule)
{
    auto line { new Line(view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kDescription), line);
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kCode), line);

    auto plain_text { new TreePlainText(view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kNote), plain_text);

    auto payment_period { new TreeSpin(0, IMAX, view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kFirst), payment_period);

    auto tax_rate { new TreeDoubleSpinPercent(&section_rule->ratio_decimal, 0, DMAX, view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kThird), tax_rate);

    auto deadline { new Deadline(DATE_D, view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kDateTime), deadline);

    auto mark { new TreeCombo(&info->unit_hash, view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kUnit), mark);

    auto branch { new CheckState(true, view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kBranch), branch);

    auto node_term { new TreeCombo(&node_term_hash_, view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kNodeRule), node_term);
}

void MainWindow::DelegateOrder(QTreeView* view, const Info* info, const SectionRule* section_rule)
{
    auto line { new Line(view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kDescription), line);

    auto total { new TreeDoubleSpinR(&section_rule->value_decimal, &info->unit_symbol_hash, view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kInitialTotal), total);
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kFinalTotal), total);
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kThird), total);
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kFourth), total);

    auto first { new TreeSpin(IMIN, IMAX, view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kFirst), first);

    auto employee { new TreeSpin(0, IMAX, view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kSecond), employee);

    auto date_time { new Deadline(DATE_D, view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kDateTime), date_time);

    auto term { new TreeCombo(&info->unit_hash, view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kUnit), term);

    auto branch { new CheckState(true, view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kBranch), branch);
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kNodeRule), branch);
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kFifth), branch);
}

void MainWindow::SetConnect(const QTreeView* view, const AbstractTreeWidget* widget, const AbstractTreeModel* model, const Sql* table_sql)
{
    connect(view, &QTreeView::doubleClicked, this, &MainWindow::RTreeViewDoubleClicked);
    connect(view, &QTreeView::customContextMenuRequested, this, &MainWindow::RTreeViewCustomContextMenuRequested);

    connect(model, &AbstractTreeModel::SUpdateName, this, &MainWindow::RUpdateName);

    connect(model, &AbstractTreeModel::SUpdateDSpinBox, widget, &AbstractTreeWidget::RUpdateDSpinBox);

    connect(model, &AbstractTreeModel::SResizeColumnToContents, view, &QTreeView::resizeColumnToContents);

    connect(table_sql, &Sql::SRemoveNode, model, &AbstractTreeModel::RRemoveNode);
    connect(table_sql, &Sql::SUpdateMultiTotal, model, &AbstractTreeModel::RUpdateMultiTotal);

    connect(table_sql, &Sql::SFreeView, this, &MainWindow::RFreeView);
}

void MainWindow::PrepInsertNode(QTreeView* view)
{
    auto current_index { view->currentIndex() };
    current_index = current_index.isValid() ? current_index : QModelIndex();

    auto parent_index { current_index.parent() };
    parent_index = parent_index.isValid() ? parent_index : QModelIndex();

    InsertNode(parent_index, current_index.row() + 1);
}

void MainWindow::InsertNode(const QModelIndex& parent, int row)
{
    auto model { section_tree_->model };
    Node* parent_node { model->GetNode(parent) };

    auto node { NodePool::Instance().Allocate() };
    node->node_rule = parent_node->node_rule;
    node->unit = parent_node->unit;
    node->parent = parent_node;

    QDialog* dialog {};

    switch (section_data_->info.section) {
    case Section::kFinance:
    case Section::kTask:
        dialog = new EditNodeFinance(node, &interface_.separator, &section_data_->info, false, false, model->Path(parent_node->id), this);
        break;
    case Section::kStakeholder:
        dialog = new EditNodeStakeholder(
            node, section_rule_, &interface_.separator, &section_data_->info, section_data_->sql, false, model->Path(parent_node->id), &node_term_hash_, this);
        break;
    case Section::kProduct:
        dialog = new EditNodeProduct(
            node, section_rule_, &interface_.separator, &section_data_->info, section_data_->sql, false, model->Path(parent_node->id), this);
        break;
    case Section::kSales:
        dialog = new EditNodeOrder(node, section_rule_, stakeholder_tree_.model->GetNodeHash(), stakeholder_tree_.model->LeafPath(),
            product_tree_.model->LeafPath(), &section_data_->info, this);
        dialog->setWindowTitle(tr(Sales));
        break;
    case Section::kPurchase:
        dialog = new EditNodeOrder(node, section_rule_, stakeholder_tree_.model->GetNodeHash(), stakeholder_tree_.model->LeafPath(),
            product_tree_.model->LeafPath(), &section_data_->info, this);
        dialog->setWindowTitle(tr(Purchase));
        break;
    default:
        return NodePool::Instance().Recycle(node);
    }

    dialog->setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
    connect(dialog, &QDialog::accepted, this, [=, this]() {
        if (model->InsertRow(row, parent, node)) {
            auto index = model->index(row, 0, parent);
            section_tree_->widget->SetCurrentIndex(index);
        }
    });
    connect(dialog, &QDialog::rejected, this, [=]() { NodePool::Instance().Recycle(node); });

    section_dialog_->append(dialog);
    dialog->show();
}

void MainWindow::AppendTrans(QWidget* widget)
{
    auto view { GetQTableView(widget) };
    auto model { GetTableModel(view) };
    int row { model->NodeRow(0) };
    QModelIndex index {};

    index = (row == -1) ? (model->AppendOne() ? model->index(model->rowCount() - 1, std::to_underlying(PartTableColumn::kDateTime)) : index)
                        : model->index(row, std::to_underlying(PartTableColumn::kRelatedNode));

    view->setCurrentIndex(index);
}

void MainWindow::RInsertTriggered()
{
    auto current_widget { ui->tabWidget->currentWidget() };
    if (!current_widget)
        return;

    if (IsTreeWidget(current_widget))
        PrepInsertNode(section_tree_->widget->View());

    if (IsTableWidget(current_widget))
        AppendTrans(current_widget);
}

void MainWindow::RRemoveTriggered()
{
    auto current_widget { ui->tabWidget->currentWidget() };
    if (!current_widget)
        return;

    if (IsTreeWidget(current_widget))
        RemoveNode(section_tree_->widget->View(), section_tree_->model);

    if (IsTableWidget(current_widget))
        DeleteTrans(current_widget);
}

void MainWindow::RemoveNode(QTreeView* view, AbstractTreeModel* model)
{
    if (!HasSelection(view))
        return;

    auto index { view->currentIndex() };
    if (!index.isValid())
        return;

    auto node_id { index.siblingAtColumn(std::to_underlying(TreeColumn::kID)).data().toInt() };
    auto node { model->GetNode(node_id) };
    if (!node)
        return;

    if (node->branch) {
        if (node->children.isEmpty())
            model->RemoveRow(index.row(), index.parent());
        else
            RemoveBranch(model, index, node_id);

        return;
    }

    auto& sql { section_data_->sql };
    bool interal_references { sql->InternalReferences(node_id) };
    bool exteral_references {};

    switch (section_data_->info.section) {
    case Section::kProduct:
        exteral_references = sql->ExternalReferences(node_id, Section::kStakeholder);
        break;
    case Section::kStakeholder:
        exteral_references = sql->ExternalReferences(node_id, Section::kSales) || sql->ExternalReferences(node_id, Section::kPurchase);
        break;
    default:
        exteral_references = false;
        break;
    }

    if (!interal_references && !exteral_references) {
        RemoveView(model, index, node_id);
        return;
    }

    if (interal_references && !exteral_references) {
        auto dialog { new class RemoveNode(model->LeafPath(), node_id, this) };
        connect(dialog, &RemoveNode::SRemoveMulti, section_data_->sql.data(), &Sql::RRemoveMulti);
        connect(dialog, &RemoveNode::SReplaceMulti, section_data_->sql.data(), &Sql::RReplaceMulti);
        dialog->exec();
        return;
    }

    auto dialog { new class RemoveNode(model->LeafPath(), node_id, this) };
    connect(dialog, &RemoveNode::SReplaceMulti, section_data_->sql.data(), &Sql::RReplaceMulti);
    dialog->DisableRemove();
    dialog->exec();
}

void MainWindow::DeleteTrans(QWidget* widget)
{
    auto view { GetQTableView(widget) };
    if (!HasSelection(view))
        return;

    auto model { GetTableModel(view) };
    auto index { view->currentIndex() };
    auto trans { model->GetTrans(index) };

    if (*trans->transport == 2)
        return;

    if (*trans->transport == 1)
        RDeleteLocation(*trans->location);

    if (model && index.isValid()) {
        int row { index.row() };
        model->DeleteOne(row);

        auto row_count { model->rowCount() };
        if (row_count == 0)
            return;

        if (row <= row_count - 1)
            index = model->index(row, 0);
        else if (row_count >= 1)
            index = model->index(row - 1, 0);

        if (index.isValid())
            view->setCurrentIndex(index);
    }
}

void MainWindow::RemoveView(AbstractTreeModel* model, const QModelIndex& index, int node_id)
{
    model->RemoveRow(index.row(), index.parent());
    auto widget { section_table_->value(node_id) };

    if (widget) {
        FreeWidget(widget);
        section_table_->remove(node_id);
        SignalStation::Instance().DeregisterModel(section_data_->info.section, node_id);
    }
}

void MainWindow::SaveTableWidget(const TableHash& table_hash, CString& section_name, CString& property)
{
    auto keys { table_hash.keys() };
    QStringList list {};

    for (const auto& node_id : keys)
        list.emplaceBack(QString::number(node_id));

    exclusive_interface_->setValue(QString("%1/%2").arg(section_name, property), list);
}

void MainWindow::RestoreTableWidget(SectionData* data, AbstractTreeModel* tree_model, const SectionRule* section_rule, TableHash* table_hash, CString& property)
{
    auto variant { exclusive_interface_->value(QString("%1/%2").arg(data->info.node, property)) };

    QList<int> list {};

    if (variant.isValid() && variant.canConvert<QStringList>()) {
        auto variant_list { variant.value<QStringList>() };
        for (const auto& node_id : variant_list)
            list.emplaceBack(node_id.toInt());
    }

    const Node* node {};

    for (const auto& node_id : list) {
        node = tree_model->GetNode(node_id);
        if (node && !node->branch)
            CreateTable(data, tree_model, section_rule, table_hash, node);
    }
}

void MainWindow::Recent()
{
    recent_list_ = shared_interface_->value(RECENT_FILE).toStringList();

    auto recent_menu { ui->menuRecent };
    QStringList valid_list {};

    for (const auto& file : recent_list_) {
        if (QFile::exists(file)) {
            auto action { recent_menu->addAction(file) };
            connect(action, &QAction::triggered, this, [file, this]() { ROpenFile(file); });
            valid_list.emplaceBack(file);
        }
    }

    if (recent_list_ != valid_list) {
        recent_list_ = valid_list;
        UpdateRecent();
    }

    SetClearMenuAction();
}

void MainWindow::RemoveBranch(AbstractTreeModel* model, const QModelIndex& index, int node_id)
{
    QMessageBox msg {};
    msg.setIcon(QMessageBox::Question);
    msg.setText(tr("Remove %1").arg(model->BranchPath()->value(node_id)));
    msg.setInformativeText(tr("The branch will be removed, and its direct children will be promoted to the same level."));
    msg.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);

    if (msg.exec() == QMessageBox::Ok)
        model->RemoveRow(index.row(), index.parent());
}

void MainWindow::RTabCloseRequested(int index)
{
    if (index == 0)
        return;

    int node_id { ui->tabWidget->tabBar()->tabData(index).value<Tab>().node_id };
    auto widget { section_table_->value(node_id) };

    FreeWidget(widget);
    section_table_->remove(node_id);

    SignalStation::Instance().DeregisterModel(section_data_->info.section, node_id);
}

template <typename T>
    requires InheritQWidget<T>
void MainWindow::FreeWidget(T*& widget)
{
    if (widget) {
        if (auto model = widget->Model()) {
            widget->SetModel(nullptr);
            delete model;
            model = nullptr;
        }

        delete widget;
        widget = nullptr;
    }
}

void MainWindow::SetTabWidget()
{
    auto tab_widget { ui->tabWidget };
    auto tab_bar { tab_widget->tabBar() };

    tab_bar->setDocumentMode(true);
    tab_bar->setExpanding(false);
    tab_bar->setTabButton(0, QTabBar::LeftSide, nullptr);

    tab_widget->setMovable(true);
    tab_widget->setTabsClosable(true);
    tab_widget->setElideMode(Qt::ElideNone);

    start_ = Section(shared_interface_->value(START_SECTION, 0).toInt());

    switch (start_) {
    case Section::kFinance:
        ui->rBtnFinance->setChecked(true);
        break;
    case Section::kStakeholder:
        ui->rBtnStakeholder->setChecked(true);
        break;
    case Section::kProduct:
        ui->rBtnProduct->setChecked(true);
        break;
    case Section::kTask:
        ui->rBtnTask->setChecked(true);
        break;
    case Section::kSales:
        ui->rBtnSales->setChecked(true);
        break;
    case Section::kPurchase:
        ui->rBtnPurchase->setChecked(true);
        break;
    default:
        break;
    }
}

void MainWindow::SetView(QTableView* view)
{
    view->setSortingEnabled(true);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setAlternatingRowColors(true);
    view->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::CurrentChanged);
    view->setColumnHidden(std::to_underlying(PartTableColumn::kID), true);

    auto h_header { view->horizontalHeader() };
    h_header->setSectionResizeMode(QHeaderView::ResizeToContents);
    h_header->setSectionResizeMode(std::to_underlying(PartTableColumn::kDescription), QHeaderView::Stretch);

    auto v_header { view->verticalHeader() };
    v_header->setDefaultSectionSize(ROW_HEIGHT);
    v_header->setSectionResizeMode(QHeaderView::Fixed);
    v_header->setHidden(true);

    view->scrollToBottom();
    view->setCurrentIndex(QModelIndex());
    view->sortByColumn(std::to_underlying(PartTableColumn::kDateTime), Qt::AscendingOrder);
}

void MainWindow::SetConnect()
{
    connect(ui->actionInsert, &QAction::triggered, this, &MainWindow::RInsertTriggered);
    connect(ui->actionRemove, &QAction::triggered, this, &MainWindow::RRemoveTriggered);
    connect(ui->actionAppend, &QAction::triggered, this, &MainWindow::RPrepAppendTriggered);
    connect(ui->actionJump, &QAction::triggered, this, &MainWindow::RJumpTriggered);
    connect(ui->actionSearch, &QAction::triggered, this, &MainWindow::RSearchTriggered);
    connect(ui->actionPreferences, &QAction::triggered, this, &MainWindow::RPreferencesTriggered);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::RAboutTriggered);
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::RNewTriggered);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::ROpenTriggered);
    connect(ui->actionClearMenu, &QAction::triggered, this, &MainWindow::RClearMenuTriggered);

    connect(ui->actionDocument, &QAction::triggered, this, &MainWindow::REditDocument);
    connect(ui->actionNode, &QAction::triggered, this, &MainWindow::REditNode);
    connect(ui->actionTransport, &QAction::triggered, this, &MainWindow::REditTransport);

    connect(ui->tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::RTabCloseRequested);
    connect(ui->tabWidget, &QTabWidget::tabBarDoubleClicked, this, &MainWindow::RTabBarDoubleClicked);

    connect(ui->actionCheckAll, &QAction::triggered, this, &MainWindow::RUpdateState);
    connect(ui->actionCheckNone, &QAction::triggered, this, &MainWindow::RUpdateState);
    connect(ui->actionCheckReverse, &QAction::triggered, this, &MainWindow::RUpdateState);
}

void MainWindow::SetFinanceData()
{
    auto section { Section::kFinance };
    auto& info { finance_data_.info };
    auto& sql { finance_data_.sql };
    auto& model { finance_tree_.model };

    info.section = section;
    info.node = FINANCE;
    info.path = FINANCE_PATH;
    info.transaction = FINANCE_TRANSACTION;

    QStringList unit_list { "CNY", "HKD", "USD", "GBP", "JPY", "CAD", "AUD", "EUR" };
    auto& unit_hash { info.unit_hash };

    QStringList unit_symbol_list { "¥", "$", "$", "£", "¥", "$", "$", "€" };
    auto& unit_symbol_hash { info.unit_symbol_hash };

    for (int i = 0; i != unit_list.size(); ++i) {
        unit_hash.insert(i, unit_list.at(i));
        unit_symbol_hash.insert(i, unit_symbol_list.at(i));
    }

    sql_.QuerySectionRule(finance_rule_, section);

    sql = QSharedPointer<Sql>::create(&info);
    finance_data_.search_sql = QSharedPointer<SearchSql>::create(&info, sql->TransactionHash());

    model = new TreeModelFinance(&info, sql, &finance_rule_, &finance_table_, &interface_, this);
    finance_tree_.widget = new TreeWidget(model, &info, &finance_rule_, this);
}

void MainWindow::SetProductData()
{
    auto section { Section::kProduct };
    auto& info { product_data_.info };
    auto& sql { product_data_.sql };
    auto& model { product_tree_.model };

    info.section = section;
    info.node = PRODUCT;
    info.path = PRODUCT_PATH;
    info.transaction = PRODUCT_TRANSACTION;

    QStringList unit_list { "", "BX", "PCS", "SET", "SF" };
    auto& unit_hash { info.unit_hash };

    for (int i = 0; i != unit_list.size(); ++i)
        unit_hash.insert(i, unit_list.at(i));

    sql_.QuerySectionRule(product_rule_, section);

    sql = QSharedPointer<SqlProduct>::create(&info);
    product_data_.search_sql = QSharedPointer<SearchSql>::create(&info, sql->TransactionHash());

    model = new TreeModelProduct(&info, sql, &product_rule_, &product_table_, &interface_, this);
    product_tree_.widget = new TreeWidget(model, &info, &product_rule_, this);
}

void MainWindow::SetStakeholderData()
{
    auto section { Section::kStakeholder };
    auto& info { stakeholder_data_.info };
    auto& sql { stakeholder_data_.sql };
    auto& model { stakeholder_tree_.model };

    info.section = section;
    info.node = STAKEHOLDER;
    info.path = STAKEHOLDER_PATH;
    info.transaction = STAKEHOLDER_TRANSACTION;

    QStringList unit_list { "E", "C", "V", "P" };
    auto& unit_hash { info.unit_hash };

    for (int i = 0; i != unit_list.size(); ++i)
        unit_hash.insert(i, unit_list.at(i));

    sql_.QuerySectionRule(stakeholder_rule_, section);

    sql = QSharedPointer<SqlStakeholder>::create(&info);
    stakeholder_data_.search_sql = QSharedPointer<SearchSql>::create(&info, sql->TransactionHash());

    stakeholder_tree_.model = new TreeModelStakeholder(&info, sql, &stakeholder_rule_, &stakeholder_table_, &interface_, this);
    stakeholder_tree_.widget = new TreeWidget(model, &info, &stakeholder_rule_, this);

    connect(product_data_.sql.data(), &Sql::SReplaceReferences, sql.data(), &Sql::RReplaceReferences);

    stakeholder_tree_.widget->HideStatus();
}

void MainWindow::SetTaskData()
{
    auto section { Section::kTask };
    auto& info { task_data_.info };
    auto& sql { task_data_.sql };
    auto& model { task_tree_.model };

    info.section = section;
    info.node = TASK;
    info.path = TASK_PATH;
    info.transaction = TASK_TRANSACTION;

    QStringList unit_list { "", "BX", "PCS", "PER", "SET", "SF" };
    auto& unit_hash { info.unit_hash };

    for (int i = 0; i != unit_list.size(); ++i)
        unit_hash.insert(i, unit_list.at(i));

    sql_.QuerySectionRule(task_rule_, section);

    sql = QSharedPointer<SqlTask>::create(&info);
    task_data_.search_sql = QSharedPointer<SearchSql>::create(&info, sql->TransactionHash());

    model = new TreeModelTask(&info, sql, &task_rule_, &task_table_, &interface_, this);
    task_tree_.widget = new TreeWidget(model, &info, &task_rule_, this);
}

void MainWindow::SetSalesData()
{
    auto section { Section::kSales };
    auto& info { sales_data_.info };
    auto& sql { sales_data_.sql };
    auto& model { sales_tree_.model };

    info.section = section;
    info.node = SALES;
    info.path = SALES_PATH;
    info.transaction = SALES_TRANSACTION;

    QStringList unit_list { "C", "M", "P" };
    auto& unit_hash { info.unit_hash };

    for (int i = 0; i != unit_list.size(); ++i)
        unit_hash.insert(i, unit_list.at(i));

    sql_.QuerySectionRule(sales_rule_, section);

    sql = QSharedPointer<SqlOrder>::create(&info);
    task_data_.search_sql = QSharedPointer<SearchSql>::create(&info, sql->TransactionHash());

    model = new TreeModelOrder(&info, sql, &sales_rule_, &sales_table_, &interface_, this);
    sales_tree_.widget = new TreeWidgetOrder(model, &info, &sales_rule_, this);

    connect(product_data_.sql.data(), &Sql::SReplaceReferences, sql.data(), &Sql::RReplaceReferences);
    connect(stakeholder_data_.sql.data(), &Sql::SReplaceReferences, sql.data(), &Sql::RReplaceReferences);
}

void MainWindow::SetPurchaseData()
{
    auto section { Section::kPurchase };
    auto& info { purchase_data_.info };
    auto& sql { purchase_data_.sql };
    auto& model { purchase_tree_.model };

    info.section = section;
    info.node = PURCHASE;
    info.path = PURCHASE_PATH;
    info.transaction = PURCHASE_TRANSACTION;

    QStringList unit_list { "C", "M", "P" };
    auto& unit_hash { info.unit_hash };

    for (int i = 0; i != unit_list.size(); ++i)
        unit_hash.insert(i, unit_list.at(i));

    sql_.QuerySectionRule(purchase_rule_, section);

    sql = QSharedPointer<SqlOrder>::create(&info);
    task_data_.search_sql = QSharedPointer<SearchSql>::create(&info, sql->TransactionHash());

    model = new TreeModelOrder(&info, sql, &purchase_rule_, &purchase_table_, &interface_, this);
    purchase_tree_.widget = new TreeWidgetOrder(model, &info, &purchase_rule_, this);

    connect(product_data_.sql.data(), &Sql::SReplaceReferences, sql.data(), &Sql::RReplaceReferences);
    connect(stakeholder_data_.sql.data(), &Sql::SReplaceReferences, sql.data(), &Sql::RReplaceReferences);
}

void MainWindow::SetDataCenter(DataCenter& data_center)
{
    data_center.info_hash.insert(Section::kFinance, &finance_data_.info);
    data_center.info_hash.insert(Section::kProduct, &product_data_.info);
    data_center.info_hash.insert(Section::kTask, &task_data_.info);
    data_center.info_hash.insert(Section::kStakeholder, &stakeholder_data_.info);

    data_center.tree_model_hash.insert(Section::kFinance, finance_tree_.model);
    data_center.tree_model_hash.insert(Section::kProduct, product_tree_.model);
    data_center.tree_model_hash.insert(Section::kTask, task_tree_.model);
    data_center.tree_model_hash.insert(Section::kStakeholder, stakeholder_tree_.model);

    data_center.leaf_path_hash.insert(Section::kFinance, finance_tree_.model->LeafPath());
    data_center.leaf_path_hash.insert(Section::kProduct, product_tree_.model->LeafPath());
    data_center.leaf_path_hash.insert(Section::kTask, task_tree_.model->LeafPath());
    data_center.leaf_path_hash.insert(Section::kStakeholder, stakeholder_tree_.model->LeafPath());

    data_center.section_rule_hash.insert(Section::kFinance, &finance_rule_);
    data_center.section_rule_hash.insert(Section::kProduct, &product_rule_);
    data_center.section_rule_hash.insert(Section::kTask, &task_rule_);
    data_center.section_rule_hash.insert(Section::kStakeholder, &stakeholder_rule_);

    data_center.sql_hash.insert(Section::kFinance, finance_data_.sql);
    data_center.sql_hash.insert(Section::kProduct, product_data_.sql);
    data_center.sql_hash.insert(Section::kTask, task_data_.sql);
    data_center.sql_hash.insert(Section::kStakeholder, stakeholder_data_.sql);
}

void MainWindow::SetHash()
{
    node_rule_hash_.insert(0, "DICD");
    node_rule_hash_.insert(1, "DDCI");

    node_term_hash_.insert(0, "C");
    node_term_hash_.insert(1, "M");

    date_format_list_.emplaceBack(DATE_TIME_FST);
}

void MainWindow::SetHeader()
{
    finance_data_.info.tree_header = { tr("Name"), tr("ID"), tr("Code"), "", "", "", "", "", "", tr("Description"), tr("Note"), tr("Rule"), tr("Branch"),
        tr("Unit"), tr("Total"), "", "" };
    finance_data_.info.part_table_header = { tr("ID"), tr("DateTime"), tr("Code"), tr("FXRate"), tr("Description"), tr("T"), tr("D"), tr("S"),
        tr("TransferNode"), tr("Debit"), tr("Credit"), tr("Balance") };
    finance_data_.info.table_header = {
        tr("ID"),
        tr("DateTime"),
        tr("Code"),
        tr("LhsNode"),
        tr("LhsFXRate"),
        tr("LhsDebit"),
        tr("LhsCredit"),
        tr("Description"),
        tr("T"),
        tr("D"),
        tr("S"),
        tr("RhsCredit"),
        tr("RhsDebit"),
        tr("RhsFXRate"),
        tr("RhsNode"),
    };

    product_data_.info.tree_header = { tr("Name"), tr("ID"), tr("Code"), "", "", tr("UnitPrice"), tr("Commission"), "", "", tr("Description"), tr("Note"),
        tr("Rule"), tr("Branch"), tr("Unit"), tr("Total"), "", "" };
    product_data_.info.part_table_header = { tr("ID"), tr("DateTime"), tr("Code"), tr("UnitCost"), tr("Description"), tr("T"), tr("D"), tr("S"), tr("Position"),
        tr("Debit"), tr("Credit"), tr("Remainder") };
    product_data_.info.table_header = {
        tr("ID"),
        tr("DateTime"),
        tr("Code"),
        tr("LhsNode"),
        tr("LhsUnitCost"),
        tr("LhsDebit"),
        tr("LhsCredit"),
        tr("Description"),
        tr("T"),
        tr("D"),
        tr("S"),
        tr("RhsCredit"),
        tr("RhsDebit"),
        tr("RhsUnitCost"),
        tr("RhsNode"),
    };

    stakeholder_data_.info.tree_header = { tr("Name"), tr("ID"), tr("Code"), tr("PaymentPeriod"), tr("Employee"), tr("TaxRate"), "", "", tr("Deadline"),
        tr("Description"), tr("Note"), tr("Term"), tr("Branch"), tr("Mark"), "", "", "" };
    stakeholder_data_.info.part_table_header
        = { tr("ID"), tr("DateTime"), tr("Code"), tr("UnitPrice"), tr("Description"), "", tr("D"), "", tr("RelatedNode"), tr("Commission"), "", "" };
    stakeholder_data_.info.table_header = {
        tr("ID"),
        tr("DateTime"),
        tr("Code"),
        tr("LhsNode"),
        tr("LhsRatio"),
        tr("LhsDebit"),
        tr("LhsCredit"),
        tr("Description"),
        tr("T"),
        tr("D"),
        tr("S"),
        tr("RhsCredit"),
        tr("RhsDebit"),
        tr("RhsRatio"),
        tr("RhsNode"),
    };

    task_data_.info.tree_header = { tr("Name"), tr("ID"), tr("Code"), "", "", "", "", "", "", tr("Description"), tr("Note"), tr("Rule"), tr("Branch"),
        tr("Unit"), tr("Total"), "", "" };
    task_data_.info.part_table_header = { tr("ID"), tr("DateTime"), tr("Code"), tr("UnitCost"), tr("Description"), tr("T"), tr("D"), tr("S"), tr("RelatedNode"),
        tr("Debit"), tr("Credit"), tr("Remainder") };
    task_data_.info.table_header = {
        tr("ID"),
        tr("DateTime"),
        tr("Code"),
        tr("LhsNode"),
        tr("LhsUnitCost"),
        tr("LhsDebit"),
        tr("LhsCredit"),
        tr("Description"),
        tr("T"),
        tr("D"),
        tr("S"),
        tr("RhsCredit"),
        tr("RhsDebit"),
        tr("RhsUnitCost"),
        tr("RhsNode"),
    };

    sales_data_.info.tree_header = { tr("Name"), tr("ID"), "", tr("First"), tr("Employee"), tr("Third"), tr("Discount"), tr("Refund"), tr("DateTime"),
        tr("Description"), "", tr("Posted"), tr("Branch"), tr("Mark"), tr("Initial Total"), tr("Final Total"), "" };

    purchase_data_.info.tree_header = { tr("Name"), tr("ID"), "", tr("First"), tr("Employee"), tr("Third"), tr("Discount"), tr("Refund"), tr("DateTime"),
        tr("Description"), "", tr("Posted"), tr("Branch"), tr("Mark"), tr("Initial Total"), tr("Final Total"), "" };
}

void MainWindow::SetAction()
{
    ui->actionInsert->setIcon(QIcon(":/solarized_dark/solarized_dark/insert.png"));
    ui->actionNode->setIcon(QIcon(":/solarized_dark/solarized_dark/edit.png"));
    ui->actionDocument->setIcon(QIcon(":/solarized_dark/solarized_dark/edit2.png"));
    ui->actionTransport->setIcon(QIcon(":/solarized_dark/solarized_dark/edit2.png"));
    ui->actionRemove->setIcon(QIcon(":/solarized_dark/solarized_dark/remove2.png"));
    ui->actionAbout->setIcon(QIcon(":/solarized_dark/solarized_dark/about.png"));
    ui->actionAppend->setIcon(QIcon(":/solarized_dark/solarized_dark/append.png"));
    ui->actionJump->setIcon(QIcon(":/solarized_dark/solarized_dark/jump.png"));
    ui->actionLocate->setIcon(QIcon(":/solarized_dark/solarized_dark/locate.png"));
    ui->actionPreferences->setIcon(QIcon(":/solarized_dark/solarized_dark/settings.png"));
    ui->actionSearch->setIcon(QIcon(":/solarized_dark/solarized_dark/search.png"));
    ui->actionNew->setIcon(QIcon(":/solarized_dark/solarized_dark/new.png"));
    ui->actionOpen->setIcon(QIcon(":/solarized_dark/solarized_dark/open.png"));
    ui->actionCheckAll->setIcon(QIcon(":/solarized_dark/solarized_dark/check-all.png"));
    ui->actionCheckNone->setIcon(QIcon(":/solarized_dark/solarized_dark/check-none.png"));
    ui->actionCheckReverse->setIcon(QIcon(":/solarized_dark/solarized_dark/check-reverse.png"));

    ui->actionCheckAll->setProperty(CHECK, std::to_underlying(Check::kAll));
    ui->actionCheckNone->setProperty(CHECK, std::to_underlying(Check::kNone));
    ui->actionCheckReverse->setProperty(CHECK, std::to_underlying(Check::kReverse));
}

void MainWindow::SetView(QTreeView* view)
{
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setDragDropMode(QAbstractItemView::InternalMove);
    view->setEditTriggers(QAbstractItemView::DoubleClicked);
    view->setDropIndicatorShown(true);
    view->setSortingEnabled(true);
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    view->setExpandsOnDoubleClick(true);

    auto header { view->header() };
    header->setSectionResizeMode(QHeaderView::ResizeToContents);
    header->setSectionResizeMode(std::to_underlying(TreeColumn::kDescription), QHeaderView::Stretch);
    header->setStretchLastSection(true);
    header->setDefaultAlignment(Qt::AlignCenter);
}

void MainWindow::HideColumn(QTreeView* view, Section section)
{
    switch (section) {
    case Section::kTask:
    case Section::kFinance:
        view->setColumnHidden(std::to_underlying(TreeColumn::kThird), true);
        view->setColumnHidden(std::to_underlying(TreeColumn::kFirst), true);
        view->setColumnHidden(std::to_underlying(TreeColumn::kSecond), true);
        view->setColumnHidden(std::to_underlying(TreeColumn::kDateTime), true);
        view->setColumnHidden(std::to_underlying(TreeColumn::kFourth), true);
        view->setColumnHidden(std::to_underlying(TreeColumn::kFinalTotal), true);
        break;
    case Section::kProduct:
        view->setColumnHidden(std::to_underlying(TreeColumn::kFirst), true);
        view->setColumnHidden(std::to_underlying(TreeColumn::kSecond), true);
        view->setColumnHidden(std::to_underlying(TreeColumn::kDateTime), true);
        view->setColumnHidden(std::to_underlying(TreeColumn::kFourth), true);
        view->setColumnHidden(std::to_underlying(TreeColumn::kFinalTotal), true);
        break;
    case Section::kStakeholder:
        view->setColumnHidden(std::to_underlying(TreeColumn::kInitialTotal), true);
        view->setColumnHidden(std::to_underlying(TreeColumn::kNodeRule), true);
        view->setColumnHidden(std::to_underlying(TreeColumn::kFourth), true);
        view->setColumnHidden(std::to_underlying(TreeColumn::kFinalTotal), true);
        view->setColumnHidden(std::to_underlying(TreeColumn::kSecond), true);
        break;
    default:
        break;
    }

    view->setColumnHidden(std::to_underlying(TreeColumn::kID), true);
}

void MainWindow::RPrepAppendTriggered()
{
    auto current_widget { ui->tabWidget->currentWidget() };
    if (!current_widget || !IsTreeWidget(current_widget))
        return;

    auto view { section_tree_->widget->View() };
    if (!HasSelection(view))
        return;

    auto parent_index { view->currentIndex() };
    if (!parent_index.isValid())
        return;

    bool branch { parent_index.siblingAtColumn(std::to_underlying(TreeColumn::kBranch)).data().toBool() };
    if (!branch)
        return;

    InsertNode(parent_index, 0);
}

void MainWindow::RJumpTriggered()
{
    auto current_widget { ui->tabWidget->currentWidget() };
    if (!current_widget || !IsTableWidget(current_widget))
        return;

    auto view { GetQTableView(current_widget) };
    if (!HasSelection(view))
        return;

    auto index { view->currentIndex() };
    if (!index.isValid())
        return;

    int row { index.row() };
    auto related_node_id { index.sibling(row, std::to_underlying(PartTableColumn::kRelatedNode)).data().toInt() };
    if (related_node_id == 0)
        return;

    if (!section_table_->contains(related_node_id)) {
        auto related_node { section_tree_->model->GetNode(related_node_id) };
        CreateTable(section_data_, section_tree_->model, section_rule_, section_table_, related_node);
    }

    auto trans_id { index.sibling(row, std::to_underlying(PartTableColumn::kID)).data().toInt() };
    SwitchTab(related_node_id, trans_id);
}

void MainWindow::RTreeViewCustomContextMenuRequested(const QPoint& pos)
{
    Q_UNUSED(pos);

    auto menu = new QMenu(this);
    menu->addAction(ui->actionInsert);
    menu->addAction(ui->actionNode);
    menu->addAction(ui->actionAppend);
    menu->addAction(ui->actionRemove);

    menu->exec(QCursor::pos());
}

void MainWindow::REditNode()
{
    auto current_widget { ui->tabWidget->currentWidget() };
    if (!current_widget || !IsTreeWidget(current_widget))
        return;

    auto view { section_tree_->widget->View() };
    if (!HasSelection(view))
        return;

    auto index { view->currentIndex() };
    if (!index.isValid())
        return;

    auto model { section_tree_->model };
    auto node { model->GetNode(index) };
    if (node->id == -1)
        return;

    auto tmp_node { new Node(*node) };
    bool node_usage { section_data_->sql->InternalReferences(node->id) };
    bool view_opened { section_table_->contains(node->id) };
    auto parent_path { model->Path(node->parent->id) };

    QDialog* dialog {};

    switch (section_data_->info.section) {
    case Section::kFinance:
    case Section::kTask:
        dialog = new EditNodeFinance(tmp_node, &interface_.separator, &section_data_->info, node_usage, view_opened, parent_path, this);
        break;
    case Section::kStakeholder:
        dialog = new EditNodeStakeholder(
            tmp_node, section_rule_, &interface_.separator, &section_data_->info, section_data_->sql, view_opened, parent_path, &node_term_hash_, this);
        break;
    case Section::kProduct:
        dialog = new EditNodeProduct(tmp_node, section_rule_, &interface_.separator, &section_data_->info, section_data_->sql, view_opened, parent_path, this);
        break;
    case Section::kSales:
        dialog = new EditNodeOrder(node, section_rule_, stakeholder_tree_.model->GetNodeHash(), stakeholder_tree_.model->LeafPath(),
            product_tree_.model->LeafPath(), &section_data_->info, this);
        dialog->setWindowTitle(tr(Sales));
        break;
    case Section::kPurchase:
        dialog = new EditNodeOrder(node, section_rule_, stakeholder_tree_.model->GetNodeHash(), stakeholder_tree_.model->LeafPath(),
            product_tree_.model->LeafPath(), &section_data_->info, this);
        dialog->setWindowTitle(tr(Purchase));
        break;
    default:
        return NodePool::Instance().Recycle(tmp_node);
    }

    dialog->setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
    connect(dialog, &QDialog::accepted, this, [=]() {
        model->UpdateNode(tmp_node);
        NodePool::Instance().Recycle(tmp_node);
    });
    connect(dialog, &QDialog::rejected, this, [=]() { NodePool::Instance().Recycle(tmp_node); });
    dialog->show();
}

template <typename T>
    requires InheritQAbstractItemView<T>
bool MainWindow::HasSelection(T* view)
{
    if (!view)
        return false;
    auto selection_model = view->selectionModel();
    return selection_model && selection_model->hasSelection();
}

void MainWindow::REditDocument()
{
    auto current_widget { ui->tabWidget->currentWidget() };
    if (!current_widget || !IsTableWidget(current_widget))
        return;

    auto view { GetQTableView(current_widget) };
    if (!HasSelection(view))
        return;

    auto trans_index { view->currentIndex() };
    if (!trans_index.isValid())
        return;

    auto document_dir { section_rule_->document_dir };
    auto model { GetTableModel(view) };
    auto trans { model->GetTrans(trans_index) };

    auto dialog { new EditDocument(section_data_->info.section, trans, document_dir, this) };
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    if (dialog->exec() == QDialog::Accepted)
        section_data_->sql->Update(section_data_->info.transaction, DOCUMENT, trans->document->join(SEMICOLON), *trans->id);
}

void MainWindow::RDeleteLocation(CStringList& location)
{
    Section section {};
    SPTransaction transaction {};
    QSharedPointer<Sql> table_sql {};
    AbstractTreeModel* tree_model {};

    int trans_id {};
    int node {};
    double ratio {};
    double debit {};
    double credit {};

    auto table_sql_hash { data_center.sql_hash };
    auto tree_model_hash { data_center.tree_model_hash };

    for (int i = 0; i != location.size(); ++i) {
        if (i % 2 == 0)
            section = Section(location.at(i).toInt());

        if (i % 2 == 1) {
            trans_id = location.at(i).toInt();

            table_sql = table_sql_hash.value(section);
            transaction = table_sql->Transaction(trans_id);
            tree_model = tree_model_hash.value(section);

            node = transaction->lhs_node;
            ratio = transaction->lhs_ratio;
            debit = transaction->lhs_debit;
            credit = transaction->lhs_credit;

            tree_model->RUpdateOneTotal(node, -ratio * debit, -ratio * credit, -debit, -credit);
            SignalStation::Instance().RRemoveOne(section, node, trans_id);

            node = transaction->rhs_node;
            ratio = transaction->rhs_ratio;
            debit = transaction->rhs_debit;
            credit = transaction->rhs_credit;

            tree_model->RUpdateOneTotal(node, -ratio * debit, -ratio * credit, -debit, -credit);
            SignalStation::Instance().RRemoveOne(section, node, trans_id);

            table_sql->Delete(trans_id);
        }
    }
}

void MainWindow::REditTransport()
{
    auto current_widget { ui->tabWidget->currentWidget() };
    if (!current_widget || !IsTableWidget(current_widget))
        return;

    auto view { GetQTableView(current_widget) };
    if (!HasSelection(view))
        return;

    auto trans_index { view->currentIndex() };
    if (!trans_index.isValid())
        return;

    auto trans { GetTableModel(view)->GetTrans(trans_index) };

    auto dialog { new EditTransport(&section_data_->info, &interface_, trans, &data_center, this) };
    dialog->setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
    connect(dialog, &EditTransport::SRetrieveOne, &SignalStation::Instance(), &SignalStation::RRetrieveOne);
    connect(dialog, &EditTransport::SDeleteOne, this, &MainWindow::RDeleteLocation);
    connect(dialog, &EditTransport::STransportLocation, this, &MainWindow::RTransportLocation);
    section_dialog_->append(dialog);
    dialog->show();
}

void MainWindow::RTransportLocation(Section section, int trans_id, int lhs_node_id, int rhs_node_id)
{
    switch (section) {
    case Section::kFinance:
        ui->rBtnFinance->setChecked(true);
        on_rBtnFinance_clicked();
        break;
    case Section::kProduct:
        ui->rBtnProduct->setChecked(true);
        on_rBtnProduct_clicked();
        break;
    case Section::kTask:
        ui->rBtnTask->setChecked(true);
        on_rBtnTask_clicked();
        break;
    default:
        break;
    }

    RTableLocation(trans_id, lhs_node_id, rhs_node_id);
}

void MainWindow::RUpdateName(const Node* node)
{
    auto model { section_tree_->model };
    int node_id { node->id };
    auto tab_bar { ui->tabWidget->tabBar() };
    int count { ui->tabWidget->count() };

    if (!node->branch) {
        if (!section_table_->contains(node_id))
            return;

        auto path { model->Path(node_id) };

        for (int index = 0; index != count; ++index) {
            if (tab_bar->tabData(index).value<Tab>().node_id == node_id) {
                tab_bar->setTabText(index, node->name);
                tab_bar->setTabToolTip(index, path);
            }
        }
    }

    if (node->branch) {
        QQueue<const Node*> queue {};
        queue.enqueue(node);

        QList<int> list {};
        while (!queue.isEmpty()) {
            auto queue_node = queue.dequeue();

            if (queue_node->branch)
                for (const auto& child : queue_node->children)
                    queue.enqueue(child);
            else
                list.emplaceBack(queue_node->id);
        }

        int node_id {};
        QString path {};

        for (int index = 0; index != count; ++index) {
            node_id = tab_bar->tabData(index).value<Tab>().node_id;

            if (list.contains(node_id)) {
                path = model->Path(node_id);
                tab_bar->setTabToolTip(index, path);
            }
        }
    }
}

void MainWindow::RUpdateSettings(const SectionRule& section_rule, const Interface& interface)
{
    if (interface_ != interface)
        UpdateInterface(interface);

    if (*section_rule_ != section_rule) {
        bool update_base_unit { section_rule_->base_unit != section_rule.base_unit };
        bool resize_column { section_rule_->value_decimal != section_rule.value_decimal || section_rule_->ratio_decimal != section_rule.ratio_decimal
            || section_rule_->hide_time != section_rule.hide_time };

        *section_rule_ = section_rule;

        if (update_base_unit) {
            auto model { section_tree_->model };
            auto node_hash { model->GetNodeHash() };
            node_hash->value(-1)->unit = section_rule.base_unit;

            for (auto node : *node_hash)
                if (node->branch && node->unit != section_rule_->base_unit)
                    model->UpdateBranchUnit(node);
        }

        section_tree_->widget->ResetStatus();

        sql_.UpdateSectionRule(section_rule, section_data_->info.section);

        if (resize_column) {
            auto current_widget { ui->tabWidget->currentWidget() };
            if (IsTableWidget(current_widget))
                ResizeColumn(GetQTableView(current_widget)->horizontalHeader(), true);
            if (IsTreeWidget(current_widget))
                ResizeColumn(section_tree_->widget->Header(), false);
        }
    }
}
void MainWindow::RFreeView(int node_id)
{
    auto view { section_table_->value(node_id) };

    if (view) {
        FreeWidget(view);
        section_table_->remove(node_id);
        SignalStation::Instance().DeregisterModel(section_data_->info.section, node_id);
    }
}

void MainWindow::UpdateInterface(const Interface& interface)
{
    auto language { interface.language };
    if (interface_.language != language) {
        if (language == EN_US) {
            qApp->removeTranslator(&cash_translator_);
            qApp->removeTranslator(&base_translator_);
        } else
            LoadAndInstallTranslator(language);

        ui->retranslateUi(this);
        SetHeader();
        UpdateTranslate();
    }

    auto separator { interface.separator };
    auto old_separator { interface_.separator };

    if (old_separator != separator) {
        if (finance_tree_.model)
            finance_tree_.model->UpdateSeparator(separator);

        if (stakeholder_tree_.model)
            stakeholder_tree_.model->UpdateSeparator(separator);

        if (product_tree_.model)
            product_tree_.model->UpdateSeparator(separator);

        auto widget { ui->tabWidget };
        int count { ui->tabWidget->count() };

        for (int index = 0; index != count; ++index)
            widget->setTabToolTip(index, widget->tabToolTip(index).replace(old_separator, separator));
    }

    interface_ = interface;

    shared_interface_->beginGroup(INTERFACE);
    shared_interface_->setValue(LANGUAGE, interface.language);
    shared_interface_->setValue(SEPARATOR, interface.separator);
    shared_interface_->setValue(DATE_FORMAT, interface.date_format);
    shared_interface_->endGroup();
}

void MainWindow::UpdateTranslate()
{
    QWidget* widget {};
    Tab tab_id {};
    auto tab_widget { ui->tabWidget };
    auto tab_bar { tab_widget->tabBar() };
    int count { tab_widget->count() };

    for (int index = 0; index != count; ++index) {
        widget = tab_widget->widget(index);
        tab_id = tab_bar->tabData(index).value<Tab>();

        if (IsTreeWidget(widget)) {
            Section section { tab_id.section };
            switch (section) {
            case Section::kFinance:
                tab_widget->setTabText(index, tr(Finance));
                break;
            case Section::kStakeholder:
                tab_widget->setTabText(index, tr(Stakeholder));
                break;
            case Section::kProduct:
                tab_widget->setTabText(index, tr(Product));
                break;
            case Section::kTask:
                tab_widget->setTabText(index, tr(Task));
                break;
            default:
                break;
            }
        }
    }
}

void MainWindow::UpdateRecent() { shared_interface_->setValue(RECENT_FILE, recent_list_); }

void MainWindow::LoadAndInstallTranslator(CString& language)
{
    QString cash_language { QString(":/I18N/I18N/") + YTX + "_" + language + SFX_QM };
    if (cash_translator_.load(cash_language))
        qApp->installTranslator(&cash_translator_);

    QString base_language { ":/I18N/I18N/qtbase_" + language + SFX_QM };
    if (base_translator_.load(base_language))
        qApp->installTranslator(&base_translator_);
}

void MainWindow::ResizeColumn(QHeaderView* header, bool table_view)
{
    header->setSectionResizeMode(QHeaderView::ResizeToContents);
    table_view ? header->setSectionResizeMode(std::to_underlying(TableColumn::kDescription), QHeaderView::Stretch)
               : header->setSectionResizeMode(std::to_underlying(TreeColumn::kDescription), QHeaderView::Stretch);
    ;
}

void MainWindow::SharedInterface(CString& dir_path)
{
    static QSettings shared_interface(dir_path + SLASH + YTX + SFX_INI, QSettings::IniFormat);
    shared_interface_ = &shared_interface;

    shared_interface.beginGroup(INTERFACE);
    interface_.language = shared_interface.value(LANGUAGE, EN_US).toString();
    interface_.theme = shared_interface.value(THEME, SOLARIZED_DARK).toString();
    interface_.date_format = shared_interface.value(DATE_FORMAT, DATE_TIME_FST).toString();
    interface_.separator = shared_interface.value(SEPARATOR, DASH).toString();
    shared_interface.endGroup();

    LoadAndInstallTranslator(interface_.language);

    QString theme { "file:///:/theme/theme/" + interface_.theme + SFX_QSS };
    qApp->setStyleSheet(theme);
}

void MainWindow::ExclusiveInterface(CString& dir_path, CString& base_name)
{
    static QSettings exclusive_interface(dir_path + SLASH + base_name + SFX_INI, QSettings::IniFormat);
    exclusive_interface_ = &exclusive_interface;
}

void MainWindow::ResourceFile()
{
    QString path {};

#ifdef Q_OS_WIN
    path = QCoreApplication::applicationDirPath() + "/resource";

    if (QDir dir(path); !dir.exists()) {
        if (!QDir::home().mkpath(path)) {
            qDebug() << "Failed to create directory:" << path;
            return;
        }
    }

    path += "/resource.brc";

#if 0
    QString command { "D:/Qt/6.7.2/llvm-mingw_64/bin/rcc.exe" };
    QStringList arguments {};
    arguments << "-binary"
              << "D:/YTX/resource/resource.qrc"
              << "-o" << path;

    QProcess process {};

    // 启动终端并执行命令
    process.start(command, arguments);
    process.waitForFinished();
#endif

#elif defined(Q_OS_MACOS)
    path = QCoreApplication::applicationDirPath() + "/../Resources/resource.brc";

#if 0
    QString command { QDir::homePath() + "/Qt/6.7.2/macos/libexec/rcc" + " -binary " + QDir::homePath() + "/Documents/YTX/resource/resource.qrc -o " + path };

    QProcess process {};
    process.start("zsh", QStringList() << "-c" << command);
    process.waitForFinished();
#endif

#endif

    QResource::registerResource(path);
}

void MainWindow::RSearchTriggered()
{
    if (!SqlConnection::Instance().DatabaseEnable())
        return;

    auto dialog { new Search(&section_data_->info, &interface_, section_tree_->model, section_data_->search_sql, section_rule_, &node_rule_hash_, this) };
    connect(dialog, &Search::STreeLocation, this, &MainWindow::RTreeLocation);
    connect(dialog, &Search::STableLocation, this, &MainWindow::RTableLocation);
    connect(section_tree_->model, &AbstractTreeModel::SSearch, dialog, &Search::RSearch);
    dialog->setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
    section_dialog_->append(dialog);
    dialog->show();
}

void MainWindow::RTreeLocation(int node_id)
{
    auto widget { section_tree_->widget };
    ui->tabWidget->setCurrentWidget(widget);

    auto index { section_tree_->model->GetIndex(node_id) };
    widget->activateWindow();
    widget->SetCurrentIndex(index);
}

void MainWindow::RTableLocation(int trans_id, int lhs_node_id, int rhs_node_id)
{
    int id { lhs_node_id };

    auto Contains = [&](int node_id) {
        if (section_table_->contains(node_id)) {
            id = node_id;
            return true;
        }
        return false;
    };

    if (!Contains(lhs_node_id) && !Contains(rhs_node_id)) {
        auto node = section_tree_->model->GetNode(lhs_node_id);
        CreateTable(section_data_, section_tree_->model, section_rule_, section_table_, node);
    }

    SwitchTab(id, trans_id);
}

void MainWindow::RPreferencesTriggered()
{
    if (!SqlConnection::Instance().DatabaseEnable())
        return;

    auto model { section_tree_->model };

    auto preference { new Preferences(&section_data_->info, model->LeafPath(), model->BranchPath(), &date_format_list_, interface_, *section_rule_, this) };
    connect(preference, &Preferences::SUpdateSettings, this, &MainWindow::RUpdateSettings);
    preference->exec();
}

void MainWindow::RAboutTriggered()
{
    static About* dialog = nullptr;

    if (!dialog) {
        dialog = new About(this);
        dialog->setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        connect(dialog, &QDialog::finished, [=]() { dialog = nullptr; });
    }

    dialog->show();
    dialog->activateWindow();
}

void MainWindow::RNewTriggered()
{
    QString filter("*.ytx");
    auto file_path { QFileDialog::getSaveFileName(this, tr("New"), QDir::homePath(), filter, nullptr) };
    if (file_path.isEmpty())
        return;

    if (!file_path.endsWith(SFX_YTX, Qt::CaseInsensitive))
        file_path += SFX_YTX;

    sql_.NewFile(file_path);
    ROpenFile(file_path);
}

void MainWindow::ROpenTriggered()
{
    QString filter("*.ytx");
    auto file_path { QFileDialog::getOpenFileName(this, tr("Open"), QDir::homePath(), filter, nullptr) };

    ROpenFile(file_path);
}

void MainWindow::SetClearMenuAction()
{
    auto menu { ui->menuRecent };

    if (!menu->isEmpty()) {
        auto separator { ui->actionSeparator };
        menu->addAction(separator);
        separator->setSeparator(true);

        menu->addAction(ui->actionClearMenu);
    }
}

template <typename T>
    requires InheritQWidget<T>
void MainWindow::SaveState(T* widget, QSettings* interface, CString& section_name, CString& property)
{
    auto state { widget->saveState() };
    interface->setValue(QString("%1/%2").arg(section_name, property), state);
}

template <typename T>
    requires InheritQWidget<T>
void MainWindow::RestoreState(T* widget, QSettings* interface, CString& section_name, CString& property)
{
    auto state { interface->value(QString("%1/%2").arg(section_name, property)).toByteArray() };

    if (!state.isEmpty())
        widget->restoreState(state);
}

template <typename T>
    requires InheritQWidget<T>
void MainWindow::SaveGeometry(T* widget, QSettings* interface, CString& section_name, CString& property)
{
    auto geometry { widget->saveGeometry() };
    interface->setValue(QString("%1/%2").arg(section_name, property), geometry);
}

template <typename T>
    requires InheritQWidget<T>
void MainWindow::RestoreGeometry(T* widget, QSettings* interface, CString& section_name, CString& property)
{
    auto geometry { interface->value(QString("%1/%2").arg(section_name, property)).toByteArray() };
    if (!geometry.isEmpty())
        widget->restoreGeometry(geometry);
}

void MainWindow::RClearMenuTriggered()
{
    ui->menuRecent->clear();
    recent_list_.clear();
    UpdateRecent();
}

void MainWindow::RTabBarDoubleClicked(int index) { RTreeLocation(ui->tabWidget->tabBar()->tabData(index).value<Tab>().node_id); }

void MainWindow::RUpdateState()
{
    auto current_widget { ui->tabWidget->currentWidget() };
    if (!current_widget || !IsTableWidget(current_widget))
        return;

    GetTableModel(GetQTableView(current_widget))->UpdateState(Check { QObject::sender()->property(CHECK).toInt() });
}

void MainWindow::on_rBtnFinance_clicked()
{
    start_ = Section::kFinance;

    if (!SqlConnection::Instance().DatabaseEnable())
        return;

    SwitchDialog(section_dialog_, false);
    UpdateLastTab();

    section_tree_ = &finance_tree_;
    section_table_ = &finance_table_;
    section_dialog_ = &finance_dialog_;
    section_rule_ = &finance_rule_;
    section_data_ = &finance_data_;

    SwitchSection(section_data_->last_tab);
}

void MainWindow::on_rBtnTask_clicked()
{
    start_ = Section::kTask;

    if (!SqlConnection::Instance().DatabaseEnable())
        return;

    SwitchDialog(section_dialog_, false);
    UpdateLastTab();

    section_tree_ = &task_tree_;
    section_table_ = &task_table_;
    section_dialog_ = &task_dialog_;
    section_rule_ = &task_rule_;
    section_data_ = &task_data_;

    SwitchSection(section_data_->last_tab);
}

void MainWindow::on_rBtnStakeholder_clicked()
{
    start_ = Section::kStakeholder;

    if (!SqlConnection::Instance().DatabaseEnable())
        return;

    SwitchDialog(section_dialog_, false);
    UpdateLastTab();

    section_tree_ = &stakeholder_tree_;
    section_table_ = &stakeholder_table_;
    section_dialog_ = &stakeholder_dialog_;
    section_rule_ = &stakeholder_rule_;
    section_data_ = &stakeholder_data_;

    SwitchSection(section_data_->last_tab);
}

void MainWindow::on_rBtnProduct_clicked()
{
    start_ = Section::kProduct;

    if (!SqlConnection::Instance().DatabaseEnable())
        return;

    SwitchDialog(section_dialog_, false);
    UpdateLastTab();

    section_tree_ = &product_tree_;
    section_table_ = &product_table_;
    section_dialog_ = &product_dialog_;
    section_rule_ = &product_rule_;
    section_data_ = &product_data_;

    SwitchSection(section_data_->last_tab);
}

void MainWindow::SwitchSection(const Tab& last_tab)
{
    auto tab_widget { ui->tabWidget };
    auto tab_bar { tab_widget->tabBar() };
    int count { tab_widget->count() };
    Tab tab {};

    for (int index = 0; index != count; ++index) {
        tab = tab_bar->tabData(index).value<Tab>();
        tab.section == start_ ? tab_widget->setTabVisible(index, true) : tab_widget->setTabVisible(index, false);

        if (tab == last_tab)
            tab_widget->setCurrentIndex(index);
    }

    SwitchDialog(section_dialog_, true);
}

void MainWindow::SwitchDialog(QList<PDialog>* dialog_list, bool enable)
{
    if (dialog_list) {
        for (auto& dialog : *dialog_list)
            if (dialog)
                dialog->setVisible(enable);
    }
}

void MainWindow::UpdateLastTab()
{
    if (section_data_) {
        auto index { ui->tabWidget->currentIndex() };
        section_data_->last_tab = ui->tabWidget->tabBar()->tabData(index).value<Tab>();
    }
}

void MainWindow::on_actionLocate_triggered()
{
    auto current_widget { ui->tabWidget->currentWidget() };
    if (!current_widget || !IsTableWidget(current_widget))
        return;

    auto view { GetQTableView(current_widget) };
    if (!HasSelection(view))
        return;

    auto index { view->currentIndex() };
    if (!index.isValid())
        return;

    int row { index.row() };
    auto transport { index.sibling(row, std::to_underlying(PartTableColumn::kTransport)).data().toString() };
    if (transport.isEmpty())
        return;

    const auto* location { GetTableModel(view)->GetTrans(index)->location };
    Section section { location->at(0).toInt() };
    int trans_id { location->at(1).toInt() };
    auto transaction { data_center.sql_hash.value(section)->Transaction(trans_id) };

    RTransportLocation(section, trans_id, transaction->lhs_node, transaction->rhs_node);
}

void MainWindow::on_rBtnSales_clicked()
{
    start_ = Section::kSales;

    if (!SqlConnection::Instance().DatabaseEnable())
        return;

    SwitchDialog(section_dialog_, false);
    UpdateLastTab();

    section_tree_ = &sales_tree_;
    section_table_ = &sales_table_;
    section_dialog_ = &sales_dialog_;
    section_rule_ = &sales_rule_;
    section_data_ = &sales_data_;

    SwitchSection(section_data_->last_tab);
}

void MainWindow::on_rBtnPurchase_clicked()
{
    start_ = Section::kPurchase;

    if (!SqlConnection::Instance().DatabaseEnable())
        return;

    SwitchDialog(section_dialog_, false);
    UpdateLastTab();

    section_tree_ = &purchase_tree_;
    section_table_ = &purchase_table_;
    section_dialog_ = &purchase_dialog_;
    section_rule_ = &purchase_rule_;
    section_data_ = &purchase_data_;

    SwitchSection(section_data_->last_tab);
}
