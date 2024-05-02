#ifndef STRUCTCLASS_H
#define STRUCTCLASS_H

#include <QDoubleSpinBox>
#include <QLabel>
#include <QPointer>
#include <QTreeView>

#include "enumclass.h"
#include "sql/searchsql.h"
#include "sql/tablesql.h"
#include "tree/treemodel.h"

struct Tab {
    Section section {};
    int node_id {};

    bool operator==(const Tab& other) const { return section == other.section && node_id == other.node_id; }
    bool operator!=(const Tab& other) const { return section != other.section || node_id != other.node_id; }
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
    Tab last_tab {};
    Info info {};
    TableSql table_sql {};
    TreeSql tree_sql {};
    SearchSql search_sql {};
};

struct DataCenter {
    QHash<Section, const Info*> info_hash {};
    QHash<Section, TreeModel*> tree_model_hash {};
    QHash<Section, TableSql*> table_sql_hash {};
    QHash<Section, const SectionRule*> section_rule_hash {};
    QHash<Section, CStringHash*> leaf_path_hash {};
};

#endif // STRUCTCLASS_H
