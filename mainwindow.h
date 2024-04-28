#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDoubleSpinBox>
#include <QLabel>
#include <QMainWindow>
#include <QPointer>
#include <QSettings>
#include <QTranslator>
#include <QTreeView>

#include "component/settings.h"
#include "component/using.h"
#include "sql/mainwindowsql.h"
#include "sql/searchsql.h"
#include "sql/tablesql.h"
#include "sql/treesql.h"
#include "table/tableinfo.h"
#include "table/tablemodel.h"
#include "tree/treemodel.h"
#include "ui_mainwindow.h"

struct Tab {
    Section section {};
    int node {};
};

struct Tree {
    QTreeView* view {};
    TreeModel* model {};
};

struct StatusBar {
    QPointer<QLabel> static_label {};
    QPointer<QDoubleSpinBox> static_spin {};
    QPointer<QLabel> dynamic_label {};
    QPointer<QDoubleSpinBox> dynamic_spin {};
};

struct Data {
    TableInfo table_info {};
    TableSql table_sql {};
    TreeInfo tree_info {};
    TreeSql tree_sql {};
    SearchSql search_sql {};
};

template <typename T>
concept InheritQAbstractItemView = std::is_base_of<QAbstractItemView, T>::value;

template <typename T>
concept InheritQWidget = std::is_base_of<QWidget, T>::value;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(CString& dir_path, QWidget* parent = nullptr);
    ~MainWindow();

signals:
    // send to all table view
    void SResizeColumnToContents(int column);

public slots:
    void ROpenFile(CString& file_path);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private slots:
    void RInsertTriggered();
    void RRemoveTriggered();
    void RPrepAppendTriggered();
    void RJumpTriggered();
    void REditTriggered();
    void RAboutTriggered();
    void RPreferencesTriggered();
    void RSearchTriggered();
    void RClearMenuTriggered();
    void RNewTriggered();
    void ROpenTriggered();

    void RSearchedNode(int node_id);
    void RSearchedTrans(int trans_id, int lhs_node_id, int rhs_node_id);

    void RUpdateDocument();
    void RUpdateSettings(const SectionRule& section_rule, const Interface& interface);
    void RUpdateName(const Node* node);
    void RUpdateStatusBarSpin();

    void RTabCloseRequested(int index);
    void RFreeView(int node_id);

    void RTreeViewCustomContextMenuRequested(const QPoint& pos);

    void RTabBarDoubleClicked(int index);
    void RTreeViewDoubleClicked(const QModelIndex& index);

    void RUpdateState();

    void on_rBtnFinance_clicked();
    void on_rBtnTask_clicked();
    void on_rBtnNetwork_clicked();
    void on_rBtnProduct_clicked();

private:
    inline bool IsTreeView(const QWidget* widget) { return widget->inherits("QTreeView"); }
    inline bool IsTableView(const QWidget* widget) { return widget->inherits("QTableView"); }
    inline TableModel* GetTableModel(QTableView* view) { return qobject_cast<TableModel*>(view->model()); }
    inline QTableView* GetTableView(QWidget* widget) { return qobject_cast<QTableView*>(widget); }

private:
    void SetTabWidget();
    void SetConnect();
    void SetHash();
    void SetHeader();
    void SetAction();
    void SetStatusBar();
    void SetClearMenuAction();

    void SetFinanceData();
    void SetProductData();
    void SetNetworkData();
    void SetTaskData();

    void CreateTable(CString& node_name, int node_id, bool node_rule);
    void CreateDelegate(QTableView* view, int node_id);
    void SetView(QTableView* view);
    void SetConnect(QTableView* view, TableModel* table, TreeModel* tree);

    Tree CreateTree(const TreeInfo* info, TreeSql* tree_sql, const SectionRule* section_rule, const Interface* interface);
    void CreateDelegate(QTreeView* view);
    void SetView(QTreeView* view);
    void SetConnect(QTreeView* view, TreeModel* model);

    void PrepInsertNode(QTreeView* view);
    void InsertNode(const QModelIndex& parent, int row);
    void AppendTrans(QWidget* widget);

    void SwitchTab(int node_id, int trans_id = 0);
    void SwitchSection(CString& name);
    bool LockFile(CString& absolute_path, CString& complete_base_name);

    void DeleteTrans(QWidget* widget);
    void RemoveNode(QTreeView* view, TreeModel* model);
    void RemoveView(TreeModel* model, const QModelIndex& index, int node_id);
    void RemoveBranch(TreeModel* model, const QModelIndex& index, int node_id);

    void UpdateInterface(const Interface& interface);
    void UpdateStatusBar();
    void UpdateTranslate();
    void UpdateRecent();

    void LoadAndInstallTranslator(CString& language);

    void SharedInterface(CString& dir_path);
    void ExclusiveInterface(CString& dir_path, CString& base_name);
    void ResourceFile();
    void Recent();

    void SaveView(const ViewHash& hash, CString& section_name, CString& property);
    void RestoreView(CString& section_name, CString& property);

    template <typename T>
        requires InheritQAbstractItemView<T>
    bool HasSelection(T* view);

    template <typename T>
        requires InheritQAbstractItemView<T>
    void FreeView(T*& view);

    template <typename T>
        requires InheritQWidget<T>
    void SaveState(T* widget, QSettings* interface, CString& section_name, CString& property);

    template <typename T>
        requires InheritQWidget<T>
    void RestoreState(T* widget, QSettings* interface, CString& section_name, CString& property);

    template <typename T>
        requires InheritQWidget<T>
    void SaveGeometry(T* widget, QSettings* interface, CString& section_name, CString& property);

    template <typename T>
        requires InheritQWidget<T>
    void RestoreGeometry(T* widget, QSettings* interface, CString& section_name, CString& property);

private:
    Ui::MainWindow* ui {};
    MainwindowSql sql_ {};

    QStringList recent_list_ {};
    Section start_ {};

    QTranslator base_translator_ {};
    QTranslator cash_translator_ {};

    QString dir_path_ {};
    QSettings* shared_interface_ {};
    QSettings* exclusive_interface_ {};

    Interface interface_ {};
    StatusBar status_bar_ {};

    StringHash node_rule_hash_ {};
    QStringList date_format_list_ {};

    Tree* section_tree_ {};
    ViewHash* section_view_ {};
    QList<QDialog*>* section_dialog_ {};
    SectionRule* section_rule_ {};
    Data* section_data_ {};

    Tree finance_tree_ {};
    ViewHash finance_view_ {};
    QList<QDialog*> finance_dialog_ {};
    SectionRule finance_rule_ {};
    Data finance_data_ {};

    Tree product_tree_ {};
    ViewHash product_view_ {};
    QList<QDialog*> product_dialog_ {};
    SectionRule product_rule_ {};
    Data product_data_ {};

    Tree network_tree_ {};
    ViewHash network_view_ {};
    QList<QDialog*> network_dialog_ {};
    SectionRule network_rule_ {};
    Data network_data_ {};

    Tree task_tree_ {};
    ViewHash task_view_ {};
    QList<QDialog*> task_dialog_ {};
    SectionRule task_rule_ {};
    Data task_data_ {};
};
#endif // MAINWINDOW_H
