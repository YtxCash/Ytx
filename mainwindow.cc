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
#include "delegate/line.h"
#include "delegate/spinbox.h"
#include "delegate/state.h"
#include "delegate/table/tabledatetime.h"
#include "delegate/table/tabledbclick.h"
#include "delegate/table/tablenodeid.h"
#include "delegate/table/tablevalue.h"
#include "delegate/tree/treecombo.h"
#include "delegate/tree/treedate.h"
#include "delegate/tree/treeplaintext.h"
#include "delegate/tree/treevaluer.h"
#include "dialog/about.h"
#include "dialog/editdocument.h"
#include "dialog/editnode.h"
#include "dialog/edittransport.h"
#include "dialog/preferences.h"
#include "dialog/removenode.h"
#include "dialog/search.h"
#include "global/nodepool.h"
#include "global/signalstation.h"
#include "global/sqlconnection.h"
#include "ui_mainwindow.h"

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
    SetStatusBar();

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

    if (finance_tree_.view) {
        SaveView(finance_view_, finance_data_.info.node, VIEW);
        SaveState(finance_tree_.view->header(), exclusive_interface_, finance_data_.info.node, HEADER_STATE);

        finance_dialog_.clear();
    }

    if (product_tree_.view) {
        SaveView(product_view_, product_data_.info.node, VIEW);
        SaveState(product_tree_.view->header(), exclusive_interface_, product_data_.info.node, HEADER_STATE);

        product_dialog_.clear();
    }

    if (network_tree_.view) {
        SaveView(network_view_, network_data_.info.node, VIEW);
        SaveState(network_tree_.view->header(), exclusive_interface_, network_data_.info.node, HEADER_STATE);

        network_dialog_.clear();
    }

    if (task_tree_.view) {
        SaveView(task_view_, task_data_.info.node, VIEW);
        SaveState(task_tree_.view->header(), exclusive_interface_, task_data_.info.node, HEADER_STATE);

        product_dialog_.clear();
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
        SetProductData();
        SetNetworkData();
        SetTaskData();

        CreateSection(finance_tree_, tr(Finance), &finance_data_, &finance_view_, &finance_rule_);
        CreateSection(network_tree_, tr(Network), &network_data_, &network_view_, &network_rule_);
        CreateSection(product_tree_, tr(Product), &product_data_, &product_view_, &product_rule_);
        CreateSection(task_tree_, tr(Task), &task_data_, &task_view_, &task_rule_);

        SetDataCenter(data_center);

        switch (start_) {
        case Section::kFinance:
            on_rBtnFinance_clicked();
            break;
        case Section::kNetwork:
            on_rBtnNetwork_clicked();
            break;
        case Section::kProduct:
            on_rBtnProduct_clicked();
            break;
        case Section::kTask:
            on_rBtnTask_clicked();
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
    if (index.column() != 0 || section_data_->info.section == Section::kNetwork)
        return;

    if (bool branch { index.siblingAtColumn(std::to_underlying(TreeColumn::kBranch)).data().toBool() })
        return;

    auto node { section_tree_->model->GetNode(index) };
    if (node->id == -1)
        return;

    if (!section_view_->contains(node->id))
        CreateTable(section_data_, section_tree_->model, section_rule_, section_view_, node);

    SwitchTab(node->id);
}

void MainWindow::SwitchTab(int node_id, int trans_id)
{
    auto view { section_view_->value(node_id, nullptr) };
    if (view) {
        ui->tabWidget->setCurrentWidget(view);
        view->activateWindow();
    }

    if (trans_id == 0)
        return;

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

void MainWindow::RUpdateStatusBarSpin()
{
    auto model { section_tree_->model };
    auto unit_symbol_hash { section_data_->info.unit_symbol_hash };
    auto static_spin { status_bar_.static_spin };
    auto dynamic_spin { status_bar_.dynamic_spin };

    auto static_node { model->GetNode(section_rule_->static_node) };
    if (static_node) {
        static_spin->setPrefix(unit_symbol_hash.value(static_node->unit, QString()));
        static_spin->setValue(static_node->foreign_total);
    }

    auto dynamic_node_lhs { model->GetNode(section_rule_->dynamic_node_lhs) };
    auto dynamic_node_rhs { model->GetNode(section_rule_->dynamic_node_rhs) };
    if (!dynamic_node_lhs || !dynamic_node_rhs)
        return;

    bool equal_unit { dynamic_node_lhs->unit == dynamic_node_rhs->unit };
    double dynamic_total {};
    auto dynamic_node_lhs_total { equal_unit ? dynamic_node_lhs->foreign_total : dynamic_node_lhs->base_total };
    auto dynamic_node_rhs_total { equal_unit ? dynamic_node_rhs->foreign_total : dynamic_node_rhs->base_total };
    auto operation { section_rule_->operation.isEmpty() ? PLUS : section_rule_->operation };

    switch (operation.at(0).toLatin1()) {
    case '+':
        dynamic_total = dynamic_node_lhs_total + dynamic_node_rhs_total;
        break;
    case '-':
        dynamic_total = dynamic_node_lhs_total - dynamic_node_rhs_total;
        break;
    default:
        dynamic_total = 0;
        break;
    }

    equal_unit ? dynamic_spin->setPrefix(unit_symbol_hash.value(dynamic_node_lhs->unit, QString()))
               : dynamic_spin->setPrefix(unit_symbol_hash.value(section_rule_->base_unit, QString()));

    dynamic_spin->setValue(dynamic_total);
}

void MainWindow::UpdateStatusBar()
{
    ui->statusbar->show();
    auto decimal { section_rule_->value_decimal };

    status_bar_.dynamic_spin->setDecimals(decimal);
    status_bar_.dynamic_spin->setValue(0.0);
    status_bar_.dynamic_spin->setPrefix(QString());
    status_bar_.static_spin->setDecimals(decimal);
    status_bar_.static_spin->setValue(0.0);
    status_bar_.static_spin->setPrefix(QString());

    status_bar_.static_label->setText(section_rule_->static_label);
    status_bar_.dynamic_label->setText(section_rule_->dynamic_label);

    RUpdateStatusBarSpin();
}

void MainWindow::CreateTable(Data* data, TreeModel* tree_model, const SectionRule* section_rule, ViewHash* view_hash, const Node* node)
{
    auto node_id { node->id };
    auto node_name { node->name };
    auto section { data->info.section };

    auto view { new QTableView(this) };
    auto model { new TableModel(&data->info, section_rule, &data->table_sql, &interface_, node_id, node->node_rule, this) };
    view->setModel(model);

    int tab_index { ui->tabWidget->addTab(view, node_name) };
    auto tab_bar { ui->tabWidget->tabBar() };

    tab_bar->setTabData(tab_index, QVariant::fromValue(Tab { section, node_id }));
    tab_bar->setTabToolTip(tab_index, tree_model->Path(node_id));

    SetView(view);
    SetConnect(view, model, tree_model, data);
    CreateDelegate(view, tree_model, section_rule, node_id);

    view_hash->insert(node_id, view);
    SignalStation::Instance().RegisterModel(section, node_id, model);
}

void MainWindow::SetConnect(const QTableView* view, const TableModel* table, const TreeModel* tree, const Data* data)
{
    connect(table, &TableModel::SResizeColumnToContents, view, &QTableView::resizeColumnToContents, Qt::UniqueConnection);

    connect(table, &TableModel::SRemoveOne, &SignalStation::Instance(), &SignalStation::RRemoveOne, Qt::UniqueConnection);
    connect(table, &TableModel::SAppendOne, &SignalStation::Instance(), &SignalStation::RAppendOne, Qt::UniqueConnection);
    connect(table, &TableModel::SUpdateBalance, &SignalStation::Instance(), &SignalStation::RUpdateBalance, Qt::UniqueConnection);
    connect(table, &TableModel::SMoveMulti, &SignalStation::Instance(), &SignalStation::RMoveMulti, Qt::UniqueConnection);

    connect(table, &TableModel::SUpdateOneTotal, tree, &TreeModel::RUpdateOneTotal, Qt::UniqueConnection);
    connect(table, &TableModel::SSearch, tree, &TreeModel::RSearch, Qt::UniqueConnection);

    connect(tree, &TreeModel::SNodeRule, table, &TableModel::RNodeRule, Qt::UniqueConnection);

    connect(&data->table_sql, &TableSql::SRemoveMulti, table, &TableModel::RRemoveMulti, Qt::UniqueConnection);

    connect(&data->table_sql, &TableSql::SMoveMulti, &SignalStation::Instance(), &SignalStation::RMoveMulti, Qt::UniqueConnection);
}

void MainWindow::CreateDelegate(QTableView* view, const TreeModel* tree_model, const SectionRule* section_rule, int node_id)
{
    auto node { new TableNodeID(tree_model->LeafPath(), node_id, view) };
    view->setItemDelegateForColumn(std::to_underlying(PartTableColumn::kRelatedNode), node);

    auto value { new TableValue(&section_rule->value_decimal, DMIN, DMAX, view) };
    view->setItemDelegateForColumn(std::to_underlying(PartTableColumn::kDebit), value);
    view->setItemDelegateForColumn(std::to_underlying(PartTableColumn::kCredit), value);
    view->setItemDelegateForColumn(std::to_underlying(PartTableColumn::kRemainder), value);

    auto ratio { new TableValue(&section_rule->ratio_decimal, DMIN, DMAX, view) };
    view->setItemDelegateForColumn(std::to_underlying(PartTableColumn::kRatio), ratio);

    auto date_time { new TableDateTime(&interface_.date_format, &section_rule->hide_time, view) };
    view->setItemDelegateForColumn(std::to_underlying(PartTableColumn::kDateTime), date_time);

    auto line { new Line(view) };
    view->setItemDelegateForColumn(std::to_underlying(PartTableColumn::kDescription), line);
    view->setItemDelegateForColumn(std::to_underlying(PartTableColumn::kCode), line);

    auto state { new State(view) };
    view->setItemDelegateForColumn(std::to_underlying(PartTableColumn::kState), state);

    auto document { new TableDbClick(view) };
    view->setItemDelegateForColumn(std::to_underlying(PartTableColumn::kDocument), document);
    connect(document, &TableDbClick::SEdit, this, &MainWindow::REditDocument, Qt::UniqueConnection);

    auto sender_receiver { new TableDbClick(view) };
    view->setItemDelegateForColumn(std::to_underlying(PartTableColumn::kTransport), sender_receiver);
    connect(sender_receiver, &TableDbClick::SEdit, this, &MainWindow::REditTransport, Qt::UniqueConnection);
}

Tree MainWindow::CreateTree(Data* data, const SectionRule* section_rule, const Interface* interface)
{
    auto view { new QTreeView(this) };
    auto model { new TreeModel(&data->info, &data->tree_sql, section_rule, section_view_, interface, this) };
    view->setModel(model);

    return { view, model };
}

void MainWindow::CreateSection(Tree& tree, CString& name, Data* data, ViewHash* view_hash, const SectionRule* section_rule)
{
    const auto* info { &data->info };
    auto tab_widget { ui->tabWidget };

    tree = CreateTree(data, section_rule, &interface_);

    auto view { tree.view };
    auto model { tree.model };

    CreateDelegate(view, info, section_rule);
    SetConnect(view, model, &data->table_sql);

    tab_widget->tabBar()->setTabData(tab_widget->addTab(view, name), QVariant::fromValue(Tab { info->section, 0 }));

    RestoreState(view->header(), exclusive_interface_, info->node, HEADER_STATE);
    RestoreView(data, model, section_rule, view_hash, VIEW);

    SetView(view);
    HideNodeColumn(view, info->section);
}

void MainWindow::CreateDelegate(QTreeView* view, const Info* info, const SectionRule* section_rule)
{
    auto line { new Line(view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kDescription), line);
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kCode), line);

    auto plain_text { new TreePlainText(view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kNote), plain_text);

    auto value { new TreeValueR(&section_rule->value_decimal, &info->unit_symbol_hash, view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kTotal), value);

    auto extension { new SpinBox(IMIN, IMAX, view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kExtension), extension);

    auto ratio { new TableValue(&section_rule->ratio_decimal, DMIN, DMAX, view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kRatio), ratio);

    auto deadline { new TreeDate(view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kDeadline), deadline);

    auto unit { new TreeCombo(&info->unit_hash, view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kUnit), unit);

    auto node_rule { new TreeCombo(&node_rule_hash_, view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kNodeRule), node_rule);

    auto branch { new State(view) };
    view->setItemDelegateForColumn(std::to_underlying(TreeColumn::kBranch), branch);
}

void MainWindow::SetConnect(const QTreeView* view, const TreeModel* model, const TableSql* table_sql)
{
    connect(view, &QTreeView::doubleClicked, this, &MainWindow::RTreeViewDoubleClicked, Qt::UniqueConnection);
    connect(view, &QTreeView::customContextMenuRequested, this, &MainWindow::RTreeViewCustomContextMenuRequested, Qt::UniqueConnection);

    connect(model, &TreeModel::SUpdateName, this, &MainWindow::RUpdateName, Qt::UniqueConnection);
    connect(model, &TreeModel::SUpdateStatusBarSpin, this, &MainWindow::RUpdateStatusBarSpin, Qt::UniqueConnection);

    connect(model, &TreeModel::SResizeColumnToContents, view, &QTreeView::resizeColumnToContents, Qt::UniqueConnection);

    connect(table_sql, &TableSql::SRemoveNode, model, &TreeModel::RRemoveNode, Qt::UniqueConnection);
    connect(table_sql, &TableSql::SUpdateMultiTotal, model, &TreeModel::RUpdateMultiTotal, Qt::UniqueConnection);

    connect(table_sql, &TableSql::SFreeView, this, &MainWindow::RFreeView, Qt::UniqueConnection);
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

    auto dialog { new EditNode(node, &interface_.separator, &section_data_->info, false, false, model->Path(parent_node->id), this) };
    if (dialog->exec() == QDialog::Rejected)
        return NodePool::Instance().Recycle(node);

    if (model->InsertRow(row, parent, node)) {
        auto index = model->index(row, 0, parent);
        section_tree_->view->setCurrentIndex(index);
    }
}

void MainWindow::AppendTrans(QWidget* widget)
{
    auto view { GetTableView(widget) };
    auto model { GetTableModel(view) };
    int row { model->NodeRow(0) };
    QModelIndex index {};

    index = (row == -1) ? (model->AppendOne() ? model->index(model->rowCount() - 1, std::to_underlying(PartTableColumn::kDateTime)) : index)
                        : model->index(row, std::to_underlying(PartTableColumn::kRelatedNode));

    view->setCurrentIndex(index);
}

void MainWindow::RInsertTriggered()
{
    auto widget { ui->tabWidget->currentWidget() };
    if (!widget)
        return;

    if (IsTreeView(widget))
        PrepInsertNode(section_tree_->view);

    if (IsTableView(widget))
        AppendTrans(ui->tabWidget->currentWidget());
}

void MainWindow::RRemoveTriggered()
{
    auto widget { ui->tabWidget->currentWidget() };
    if (!widget)
        return;

    if (IsTreeView(widget))
        RemoveNode(section_tree_->view, section_tree_->model);

    if (IsTableView(widget))
        DeleteTrans(ui->tabWidget->currentWidget());
}

void MainWindow::RemoveNode(QTreeView* view, TreeModel* model)
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

    if (!section_data_->tree_sql.Usage(node_id)) {
        RemoveView(model, index, node_id);
        return;
    }

    auto dialog { new class RemoveNode(model->LeafPath(), node_id, this) };

    connect(dialog, &RemoveNode::SRemoveMulti, &section_data_->table_sql, &TableSql::RRemoveMulti, Qt::UniqueConnection);
    connect(dialog, &RemoveNode::SReplaceMulti, &section_data_->table_sql, &TableSql::RReplaceMulti, Qt::UniqueConnection);

    dialog->exec();
}

void MainWindow::DeleteTrans(QWidget* widget)
{
    auto view { GetTableView(widget) };
    if (!HasSelection(view))
        return;

    auto model { GetTableModel(view) };
    auto index { view->currentIndex() };
    auto trans { model->GetTrans(index) };

    if (*trans->transport == -1)
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

void MainWindow::RemoveView(TreeModel* model, const QModelIndex& index, int node_id)
{
    model->RemoveRow(index.row(), index.parent());
    auto view { section_view_->value(node_id) };

    if (view) {
        FreeView(view);
        section_view_->remove(node_id);
        SignalStation::Instance().DeregisterModel(section_data_->info.section, node_id);
    }
}

void MainWindow::SaveView(const ViewHash& hash, CString& section_name, CString& property)
{
    auto keys { hash.keys() };
    QStringList list {};

    for (const int& node_id : keys)
        list.emplaceBack(QString::number(node_id));

    exclusive_interface_->setValue(QString("%1/%2").arg(section_name, property), list);
}

void MainWindow::RestoreView(Data* data, TreeModel* tree_model, const SectionRule* section_rule, ViewHash* view_hash, CString& property)
{
    auto variant { exclusive_interface_->value(QString("%1/%2").arg(data->info.node, property)) };

    QList<int> list {};

    if (variant.isValid() && variant.canConvert<QStringList>()) {
        auto variant_list { variant.value<QStringList>() };
        for (const auto& node_id : variant_list)
            list.emplaceBack(node_id.toInt());
    }

    const Node* node {};

    for (const int& node_id : list) {
        node = tree_model->GetNode(node_id);
        if (node && !node->branch)
            CreateTable(data, tree_model, section_rule, view_hash, node);
    }
}

void MainWindow::Recent()
{
    recent_list_ = shared_interface_->value(RECENT_FILE).toStringList();

    auto recent_menu { ui->menuRecent };
    QStringList valid_list {};

    for (auto& file : recent_list_) {
        if (QFile::exists(file)) {
            auto action { recent_menu->addAction(file) };
            connect(action, &QAction::triggered, [file, this]() { ROpenFile(file); });
            valid_list.emplaceBack(file);
        }
    }

    if (recent_list_ != valid_list) {
        recent_list_ = valid_list;
        UpdateRecent();
    }

    SetClearMenuAction();
}

void MainWindow::HideStatusBar(Section section)
{
    switch (section) {
    case Section::kNetwork:
        status_bar_.dynamic_spin->hide();
        status_bar_.static_spin->hide();
        break;
    default:
        status_bar_.dynamic_spin->show();
        status_bar_.static_spin->show();
        break;
    }
}

void MainWindow::RemoveBranch(TreeModel* model, const QModelIndex& index, int node_id)
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
    auto view { section_view_->value(node_id) };

    FreeView(view);
    section_view_->remove(node_id);

    SignalStation::Instance().DeregisterModel(section_data_->info.section, node_id);
}

template <typename T>
    requires InheritQAbstractItemView<T>
void MainWindow::FreeView(T*& view)
{
    if (view) {
        if (auto model = view->model()) {
            view->setModel(nullptr);
            delete model;
            model = nullptr;
        }

        delete view;
        view = nullptr;
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

    ui->rBtnPurchase->setHidden(true);
    ui->rBtnSales->setHidden(true);
    ui->splitter->setCollapsible(1, true);

    start_ = Section(shared_interface_->value(START_SECTION, 0).toInt());

    switch (start_) {
    case Section::kFinance:
        ui->rBtnFinance->setChecked(true);
        break;
    case Section::kNetwork:
        ui->rBtnNetwork->setChecked(true);
        break;
    case Section::kProduct:
        ui->rBtnProduct->setChecked(true);
        break;
    case Section::kTask:
        ui->rBtnTask->setChecked(true);
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
    connect(ui->actionInsert, &QAction::triggered, this, &MainWindow::RInsertTriggered, Qt::UniqueConnection);
    connect(ui->actionRemove, &QAction::triggered, this, &MainWindow::RRemoveTriggered, Qt::UniqueConnection);
    connect(ui->actionAppend, &QAction::triggered, this, &MainWindow::RPrepAppendTriggered, Qt::UniqueConnection);
    connect(ui->actionJump, &QAction::triggered, this, &MainWindow::RJumpTriggered, Qt::UniqueConnection);
    connect(ui->actionSearch, &QAction::triggered, this, &MainWindow::RSearchTriggered, Qt::UniqueConnection);
    connect(ui->actionPreferences, &QAction::triggered, this, &MainWindow::RPreferencesTriggered, Qt::UniqueConnection);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::RAboutTriggered, Qt::UniqueConnection);
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::RNewTriggered, Qt::UniqueConnection);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::ROpenTriggered, Qt::UniqueConnection);
    connect(ui->actionClearMenu, &QAction::triggered, this, &MainWindow::RClearMenuTriggered, Qt::UniqueConnection);

    connect(ui->actionDocument, &QAction::triggered, this, &MainWindow::REditDocument, Qt::UniqueConnection);
    connect(ui->actionNode, &QAction::triggered, this, &MainWindow::REditNode, Qt::UniqueConnection);
    connect(ui->actionTransport, &QAction::triggered, this, &MainWindow::REditTransport, Qt::UniqueConnection);

    connect(ui->tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::RTabCloseRequested, Qt::UniqueConnection);
    connect(ui->tabWidget, &QTabWidget::tabBarDoubleClicked, this, &MainWindow::RTabBarDoubleClicked, Qt::UniqueConnection);

    connect(ui->actionCheckAll, &QAction::triggered, this, &MainWindow::RUpdateState, Qt::UniqueConnection);
    connect(ui->actionCheckNone, &QAction::triggered, this, &MainWindow::RUpdateState, Qt::UniqueConnection);
    connect(ui->actionCheckReverse, &QAction::triggered, this, &MainWindow::RUpdateState, Qt::UniqueConnection);
}

void MainWindow::SetFinanceData()
{
    auto section { Section::kFinance };
    auto& info { finance_data_.info };

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

    finance_data_.tree_sql = TreeSql(&info);
    finance_data_.table_sql.SetInfo(&info);
    finance_data_.search_sql = SearchSql(&info, finance_data_.table_sql.TransactionHash());
}

void MainWindow::SetProductData()
{
    auto section { Section::kProduct };
    auto& info { product_data_.info };

    info.section = section;
    info.node = PRODUCT;
    info.path = PRODUCT_PATH;
    info.transaction = PRODUCT_TRANSACTION;

    QStringList unit_list { QString(), "PCS", "SET", "SF", "BX" };
    auto& unit_hash { info.unit_hash };

    for (int i = 0; i != unit_list.size(); ++i)
        unit_hash.insert(i, unit_list.at(i));

    sql_.QuerySectionRule(product_rule_, section);

    product_data_.tree_sql = TreeSql(&info);
    product_data_.table_sql.SetInfo(&info);
    product_data_.search_sql = SearchSql(&info, product_data_.table_sql.TransactionHash());
}

void MainWindow::SetNetworkData()
{
    auto section { Section::kNetwork };
    auto& info { network_data_.info };

    info.section = section;
    info.node = NETWORK;
    info.path = NETWORK_PATH;
    info.transaction = NETWORK_TRANSACTION;

    QStringList unit_list { QString(), "C", "V", "E" };
    auto& unit_hash { info.unit_hash };

    for (int i = 0; i != unit_list.size(); ++i)
        unit_hash.insert(i, unit_list.at(i));

    sql_.QuerySectionRule(network_rule_, section);

    network_data_.tree_sql = TreeSql(&info);
    network_data_.table_sql.SetInfo(&info);
    network_data_.search_sql = SearchSql(&info, network_data_.table_sql.TransactionHash());
}

void MainWindow::SetTaskData()
{
    auto section { Section::kTask };
    auto& info { task_data_.info };

    info.section = section;
    info.node = TASK;
    info.path = TASK_PATH;
    info.transaction = TASK_TRANSACTION;

    QStringList unit_list { QString(), "PER", "SF", "PCS", "SET", "BX" };
    auto& unit_hash { info.unit_hash };

    for (int i = 0; i != unit_list.size(); ++i)
        unit_hash.insert(i, unit_list.at(i));

    sql_.QuerySectionRule(task_rule_, section);

    task_data_.tree_sql = TreeSql(&info);
    task_data_.table_sql.SetInfo(&info);
    task_data_.search_sql = SearchSql(&info, task_data_.table_sql.TransactionHash());
}

void MainWindow::SetDataCenter(DataCenter& data_center)
{
    data_center.info_hash.insert(Section::kFinance, &finance_data_.info);
    data_center.info_hash.insert(Section::kProduct, &product_data_.info);
    data_center.info_hash.insert(Section::kTask, &task_data_.info);
    data_center.info_hash.insert(Section::kNetwork, &network_data_.info);

    data_center.tree_model_hash.insert(Section::kFinance, finance_tree_.model);
    data_center.tree_model_hash.insert(Section::kProduct, product_tree_.model);
    data_center.tree_model_hash.insert(Section::kTask, task_tree_.model);
    data_center.tree_model_hash.insert(Section::kNetwork, network_tree_.model);

    data_center.leaf_path_hash.insert(Section::kFinance, finance_tree_.model->LeafPath());
    data_center.leaf_path_hash.insert(Section::kProduct, product_tree_.model->LeafPath());
    data_center.leaf_path_hash.insert(Section::kTask, task_tree_.model->LeafPath());
    data_center.leaf_path_hash.insert(Section::kNetwork, network_tree_.model->LeafPath());

    data_center.section_rule_hash.insert(Section::kFinance, &finance_rule_);
    data_center.section_rule_hash.insert(Section::kProduct, &product_rule_);
    data_center.section_rule_hash.insert(Section::kTask, &task_rule_);
    data_center.section_rule_hash.insert(Section::kNetwork, &network_rule_);

    data_center.table_sql_hash.insert(Section::kFinance, &finance_data_.table_sql);
    data_center.table_sql_hash.insert(Section::kProduct, &product_data_.table_sql);
    data_center.table_sql_hash.insert(Section::kTask, &task_data_.table_sql);
    data_center.table_sql_hash.insert(Section::kNetwork, &network_data_.table_sql);
}

void MainWindow::SetHash()
{
    node_rule_hash_.insert(0, "DICD");
    node_rule_hash_.insert(1, "DDCI");

    date_format_list_.emplaceBack(DATE_TIME_FST);
}

void MainWindow::SetHeader()
{
    finance_data_.info.tree_header = { tr("Account"), tr("ID"), tr("Code"), QString(), QString(), QString(), tr("Description"), tr("Note"), tr("R"), tr("B"),
        tr("U"), tr("Total"), QString() };
    finance_data_.info.partial_table_header = { tr("ID"), tr("DateTime"), tr("Code"), tr("FXRate"), tr("Description"), tr("T"), tr("D"), tr("S"),
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

    product_data_.info.tree_header = { tr("Variety"), tr("ID"), tr("Code"), QString(), tr("UnitPrice"), QString(), tr("Description"), tr("Note"), tr("R"),
        tr("B"), tr("U"), tr("Total"), QString() };
    product_data_.info.partial_table_header = { tr("ID"), tr("DateTime"), tr("Code"), tr("UnitCost"), tr("Description"), tr("T"), tr("D"), tr("S"),
        tr("Position"), tr("Debit"), tr("Credit"), tr("Remainder") };
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

    network_data_.info.tree_header = { tr("Name"), tr("ID"), tr("Code"), tr("PaymentTerm"), tr("TaxRate"), tr("Deadline"), tr("Description"), tr("Note"),
        tr("R"), tr("B"), tr("U"), tr("Total"), QString() };
    network_data_.info.partial_table_header = { tr("ID"), tr("DateTime"), tr("Code"), tr("Ratio"), tr("Description"), tr("T"), tr("D"), tr("S"),
        tr("RelatedNode"), tr("Debit"), tr("Credit"), tr("Remainder") };
    network_data_.info.table_header = {
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

    task_data_.info.tree_header = { tr("Variety"), tr("ID"), tr("Code"), QString(), tr("UnitPrice"), QString(), tr("Description"), tr("Note"), tr("R"), tr("B"),
        tr("U"), tr("Total"), QString() };
    task_data_.info.partial_table_header = { tr("ID"), tr("DateTime"), tr("Code"), tr("UnitCost"), tr("Description"), tr("T"), tr("D"), tr("S"),
        tr("RelatedNode"), tr("Debit"), tr("Credit"), tr("Remainder") };
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
    view->setColumnHidden(std::to_underlying(TreeColumn::kID), true);
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    view->setExpandsOnDoubleClick(true);

    auto header { view->header() };
    header->setSectionResizeMode(QHeaderView::ResizeToContents);
    header->setSectionResizeMode(std::to_underlying(TreeColumn::kDescription), QHeaderView::Stretch);
    header->setStretchLastSection(true);
}

void MainWindow::HideNodeColumn(QTreeView* view, Section section)
{
    switch (section) {
    case Section::kFinance:
        view->setColumnHidden(std::to_underlying(TreeColumn::kRatio), true);
        view->setColumnHidden(std::to_underlying(TreeColumn::kExtension), true);
        view->setColumnHidden(std::to_underlying(TreeColumn::kDeadline), true);
        break;
    case Section::kTask:
        view->setColumnHidden(std::to_underlying(TreeColumn::kRatio), true);
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

void MainWindow::RPrepAppendTriggered()
{
    auto widget { ui->tabWidget->currentWidget() };
    if (!widget || !IsTreeView(widget))
        return;

    auto view { section_tree_->view };
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
    auto widget { ui->tabWidget->currentWidget() };
    if (!widget || !IsTableView(widget))
        return;

    auto view { GetTableView(widget) };
    if (!HasSelection(view))
        return;

    auto index { view->currentIndex() };
    if (!index.isValid())
        return;

    int row { index.row() };
    auto related_node_id { index.sibling(row, std::to_underlying(PartTableColumn::kRelatedNode)).data().toInt() };
    if (related_node_id == 0)
        return;

    if (!section_view_->contains(related_node_id)) {
        auto related_node { section_tree_->model->GetNode(related_node_id) };
        CreateTable(section_data_, section_tree_->model, section_rule_, section_view_, related_node);
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
    auto widget { ui->tabWidget->currentWidget() };
    if (!widget || !IsTreeView(widget))
        return;

    auto view { section_tree_->view };
    if (!HasSelection(view))
        return;

    auto index { view->currentIndex() };
    if (!index.isValid())
        return;

    auto model { section_tree_->model };
    auto node { model->GetNode(index) };
    if (node->id == -1)
        return;

    Node tmp_node { *node };
    bool node_usage { section_data_->tree_sql.Usage(node->id) };
    bool view_opened { section_view_->contains(node->id) };
    auto parent_path { model->Path(node->parent->id) };

    auto dialog { new EditNode(&tmp_node, &interface_.separator, &section_data_->info, node_usage, view_opened, parent_path, this) };
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    if (dialog->exec() == QDialog::Accepted)
        model->UpdateNode(&tmp_node);
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
    auto widget { ui->tabWidget->currentWidget() };
    if (!widget || !IsTableView(widget))
        return;

    auto view { GetTableView(widget) };
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
        section_data_->table_sql.Update(DOCUMENT, trans->document->join(SEMICOLON), *trans->id);
}

void MainWindow::RDeleteLocation(CStringList& location)
{
    Section section {};
    SPTransaction transaction {};
    TableSql* table_sql {};
    TreeModel* tree_model {};

    int trans_id {};
    int node {};
    double ratio {};
    double debit {};
    double credit {};

    auto table_sql_hash { data_center.table_sql_hash };
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
    auto widget { ui->tabWidget->currentWidget() };
    if (!widget || !IsTableView(widget))
        return;

    auto view { GetTableView(widget) };
    if (!HasSelection(view))
        return;

    auto trans_index { view->currentIndex() };
    if (!trans_index.isValid())
        return;

    auto trans { GetTableModel(view)->GetTrans(trans_index) };

    auto dialog { new EditTransport(&section_data_->info, &interface_, trans, &data_center, this) };
    dialog->setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(dialog, &EditTransport::SRetrieveOne, &SignalStation::Instance(), &SignalStation::RRetrieveOne, Qt::UniqueConnection);
    connect(dialog, &EditTransport::SDeleteOne, this, &MainWindow::RDeleteLocation, Qt::UniqueConnection);
    connect(dialog, &EditTransport::STransportLocation, this, &MainWindow::RTransportLocation, Qt::UniqueConnection);
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
        if (!section_view_->contains(node_id))
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
                for (const auto* child : queue_node->children)
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
        bool update_base_unit { section_rule_->base_unit == section_rule.base_unit ? false : true };
        *section_rule_ = section_rule;

        if (update_base_unit) {
            auto model { section_tree_->model };
            auto node_hash { model->GetNodeHash() };
            node_hash->value(-1)->unit = section_rule.base_unit;

            for (auto node : *node_hash)
                model->UpdateBranchUnit(node);
        }

        UpdateStatusBar();
        sql_.UpdateSectionRule(section_rule, section_data_->info.section);

        emit SResizeColumnToContents(std::to_underlying(PartTableColumn::kDateTime));
        emit SResizeColumnToContents(std::to_underlying(PartTableColumn::kDebit));
        emit SResizeColumnToContents(std::to_underlying(PartTableColumn::kCredit));
        emit SResizeColumnToContents(std::to_underlying(PartTableColumn::kRemainder));
        emit SResizeColumnToContents(std::to_underlying(PartTableColumn::kRatio));
    }
}
void MainWindow::RFreeView(int node_id)
{
    auto view { section_view_->value(node_id) };

    if (view) {
        FreeView(view);
        section_view_->remove(node_id);
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

        if (network_tree_.model)
            network_tree_.model->UpdateSeparator(separator);

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

        if (IsTreeView(widget)) {
            Section section { tab_id.section };
            switch (section) {
            case Section::kFinance:
                tab_widget->setTabText(index, tr(Finance));
                break;
            case Section::kNetwork:
                tab_widget->setTabText(index, tr(Network));
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
    QString command { "D:/Qt/6.5.3/mingw_64/bin/rcc.exe" };
    QStringList arguments {};
    arguments << "-binary" << "E:/YTX/resource/resource.qrc" << "-o" << path;

    QProcess process {};

    // 启动终端并执行命令
    process.start(command, arguments);
    process.waitForFinished();
#endif

#elif defined(Q_OS_MACOS)
    path = QCoreApplication::applicationDirPath() + "/../Resources/resource.brc";

#if 0
    QString command { QDir::homePath() + "/Qt6.5.3/6.5.3/macos/libexec/rcc" + " -binary " + QDir::homePath() + "/Documents/Ytx/resource/resource.qrc -o "
        + path };

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

    auto dialog { new Search(&section_data_->info, &interface_, section_tree_->model, &section_data_->search_sql, section_rule_, &node_rule_hash_, this) };
    connect(dialog, &Search::STreeLocation, this, &MainWindow::RTreeLocation, Qt::UniqueConnection);
    connect(dialog, &Search::STableLocation, this, &MainWindow::RTableLocation, Qt::UniqueConnection);
    connect(section_tree_->model, &TreeModel::SSearch, dialog, &Search::RSearch, Qt::UniqueConnection);
    dialog->setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    section_dialog_->append(dialog);
    dialog->show();
}

void MainWindow::RTreeLocation(int node_id)
{
    auto view { section_tree_->view };
    ui->tabWidget->setCurrentWidget(view);

    auto index { section_tree_->model->GetIndex(node_id) };
    view->activateWindow();
    view->setCurrentIndex(index);
}

void MainWindow::RTableLocation(int trans_id, int lhs_node_id, int rhs_node_id)
{
    int id { lhs_node_id };

    auto Contains = [&](int node_id) {
        if (section_view_->contains(node_id)) {
            id = node_id;
            return true;
        }
        return false;
    };

    if (!Contains(lhs_node_id) && !Contains(rhs_node_id)) {
        auto node = section_tree_->model->GetNode(lhs_node_id);
        CreateTable(section_data_, section_tree_->model, section_rule_, section_view_, node);
    }

    SwitchTab(id, trans_id);
}

void MainWindow::RPreferencesTriggered()
{
    if (!SqlConnection::Instance().DatabaseEnable())
        return;

    auto model { section_tree_->model };

    auto preference { new Preferences(&section_data_->info, model->LeafPath(), model->BranchPath(), &date_format_list_, interface_, *section_rule_, this) };
    connect(preference, &Preferences::SUpdateSettings, this, &MainWindow::RUpdateSettings, Qt::UniqueConnection);
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

void MainWindow::SetStatusBar()
{
    status_bar_.static_label = new QLabel(this);
    status_bar_.static_spin = new QDoubleSpinBox(this);
    status_bar_.dynamic_label = new QLabel(this);
    status_bar_.dynamic_spin = new QDoubleSpinBox(this);

    status_bar_.static_spin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    status_bar_.static_spin->setRange(DMIN, DMAX);
    status_bar_.static_spin->setReadOnly(true);
    status_bar_.static_spin->setGroupSeparatorShown(true);
    status_bar_.static_spin->setFocusPolicy(Qt::NoFocus);

    status_bar_.dynamic_spin->setRange(DMIN, DMAX);
    status_bar_.dynamic_spin->setButtonSymbols(QAbstractSpinBox::NoButtons);
    status_bar_.dynamic_spin->setReadOnly(true);
    status_bar_.dynamic_spin->setGroupSeparatorShown(true);
    status_bar_.dynamic_spin->setFocusPolicy(Qt::NoFocus);

    auto space_before_middle { new QWidget(this) };
    space_before_middle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    auto space_after_middle { new QWidget(this) };
    space_after_middle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    ui->statusbar->addPermanentWidget(space_before_middle);
    ui->statusbar->addPermanentWidget(status_bar_.static_label);
    ui->statusbar->addPermanentWidget(status_bar_.static_spin);
    ui->statusbar->addPermanentWidget(space_after_middle);
    ui->statusbar->addPermanentWidget(status_bar_.dynamic_label);
    ui->statusbar->addPermanentWidget(status_bar_.dynamic_spin);

    ui->statusbar->hide();
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
    auto widget { ui->tabWidget->currentWidget() };
    if (!widget || !IsTableView(widget))
        return;

    GetTableModel(GetTableView(widget))->UpdateState(Check { QObject::sender()->property(CHECK).toInt() });
}

void MainWindow::on_rBtnFinance_clicked()
{
    start_ = Section::kFinance;

    if (!SqlConnection::Instance().DatabaseEnable())
        return;

    SwitchDialog(section_dialog_, false);
    SwitchTab();

    section_tree_ = &finance_tree_;
    section_view_ = &finance_view_;
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
    SwitchTab();

    section_tree_ = &task_tree_;
    section_view_ = &task_view_;
    section_dialog_ = &task_dialog_;
    section_rule_ = &task_rule_;
    section_data_ = &task_data_;

    SwitchSection(section_data_->last_tab);
}

void MainWindow::on_rBtnNetwork_clicked()
{
    start_ = Section::kNetwork;

    if (!SqlConnection::Instance().DatabaseEnable())
        return;

    SwitchDialog(section_dialog_, false);
    SwitchTab();

    section_tree_ = &network_tree_;
    section_view_ = &network_view_;
    section_dialog_ = &network_dialog_;
    section_rule_ = &network_rule_;
    section_data_ = &network_data_;

    SwitchSection(section_data_->last_tab);
}

void MainWindow::on_rBtnProduct_clicked()
{
    start_ = Section::kProduct;

    if (!SqlConnection::Instance().DatabaseEnable())
        return;

    SwitchDialog(section_dialog_, false);
    SwitchTab();

    section_tree_ = &product_tree_;
    section_view_ = &product_view_;
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
    UpdateStatusBar();
    HideStatusBar(section_data_->info.section);
}

void MainWindow::SwitchDialog(QList<PDialog>* dialog_list, bool enable)
{
    if (dialog_list) {
        for (auto dialog : *dialog_list)
            if (dialog)
                dialog->setVisible(enable);
    }
}

void MainWindow::SwitchTab()
{
    if (section_data_) {
        auto index { ui->tabWidget->currentIndex() };
        section_data_->last_tab = ui->tabWidget->tabBar()->tabData(index).value<Tab>();
    }
}

void MainWindow::on_actionLocate_triggered()
{
    auto widget { ui->tabWidget->currentWidget() };
    if (!widget || !IsTableView(widget))
        return;

    auto view { GetTableView(widget) };
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
    auto transaction { data_center.table_sql_hash.value(section)->Transaction(trans_id) };

    RTransportLocation(section, trans_id, transaction->lhs_node, transaction->rhs_node);
}
