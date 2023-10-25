#ifndef SEARCH_H
#define SEARCH_H

#include <QDialog>
#include <QTableView>

#include "component/using.h"
#include "sql/searchsql.h"
#include "table/searchnodemodel.h"
#include "table/searchtransmodel.h"

namespace Ui {
class Search;
}

class Search : public QDialog {
    Q_OBJECT

public:
    Search(const TreeInfo* tree_info, const TreeModel* tree_model, const TableInfo* table_info, SearchSql* sql, const SectionRule* section_rule,
        CStringHash* unit_hash, CStringHash* rule_hash, CStringHash* unit_symbol_hash, QWidget* parent = nullptr);
    ~Search();

signals:
    void SSearchedNode(int node_id);
    void SSearchedTrans(int trans_id, int lhs_node_id, int rhs_node_id);

public slots:
    void RSearch();

private slots:
    void RDoubleClicked(const QModelIndex& index);

    void on_rBtnNode_toggled(bool checked);
    void on_rBtnTransaction_toggled(bool checked);

private:
    void IniDialog();
    void IniConnect();

    void IniNode(QTableView* view, SearchNodeModel* model);
    void IniTrans(QTableView* view, SearchTransModel* model);

    void IniView(QTableView* view);

private:
    Ui::Search* ui;
    SearchNodeModel* node_model_ {};
    SearchTransModel* trans_model_ {};

    SearchSql* sql_ {};

    CStringHash* unit_hash_ {};
    CStringHash* node_rule_hash_ {};
    const SectionRule* section_rule_ {};
    const TreeModel* tree_model_ {};
    const TreeInfo* tree_info_ {};
    CStringHash* unit_symbol_hash_ {};
};

#endif // SEARCH_H
