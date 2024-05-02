#ifndef SEARCH_H
#define SEARCH_H

#include <QDialog>
#include <QTableView>

#include "component/using.h"
#include "sql/searchsql.h"
#include "table/searchtablemodel.h"
#include "table/searchtreemodel.h"

namespace Ui {
class Search;
}

class Search : public QDialog {
    Q_OBJECT

public:
    Search(const Info* info, const Interface* interface, const TreeModel* tree_model, SearchSql* sql, const SectionRule* section_rule,
        const CStringHash* node_rule, QWidget* parent = nullptr);
    ~Search();

signals:
    void STreeLocation(int node_id);
    void STableLocation(int trans_id, int lhs_node_id, int rhs_node_id);

public slots:
    void RSearch();

private slots:
    void RDoubleClicked(const QModelIndex& index);

    void on_rBtnNode_toggled(bool checked);
    void on_rBtnTransaction_toggled(bool checked);

private:
    void IniDialog();
    void IniConnect();

    void IniTree(QTableView* view, SearchTreeModel* model);
    void IniTable(QTableView* view, SearchTableModel* model);

    void IniView(QTableView* view);
    void HideColumn(QTableView* view, Section section);

    void ResizeTreeColumn(QHeaderView* header);
    void ResizeTableColumn(QHeaderView* header);

private:
    Ui::Search* ui;
    SearchTreeModel* search_tree_model_ {};
    SearchTableModel* search_table_model_ {};

    SearchSql* sql_ {};

    const CStringHash* node_rule_ {};
    const SectionRule* section_rule_ {};
    const TreeModel* tree_model_ {};
    const Info* info_ {};
    const Interface* interface_ {};
};

#endif // SEARCH_H
