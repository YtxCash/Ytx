#ifndef DATA_H
#define DATA_H

#include "component/settings.h"
#include "enumclass.h"
#include "sql/searchsql.h"
#include "sql/sql.h"
#include "tree/abstracttreemodel.h"
#include "widget/abstracttreewidget.h"

struct Tab {
    Section section {};
    int node_id {};

    // Equality operator overload to compare two Tab structs
    bool operator==(const Tab& other) const noexcept { return std::tie(section, node_id) == std::tie(other.section, other.node_id); }

    // Inequality operator overload to compare two Tab structs
    bool operator!=(const Tab& other) const noexcept { return !(*this == other); }
};

struct Tree {
    AbstractTreeWidget* widget {};
    AbstractTreeModel* model {};
};

struct SectionData {
    Tab last_tab {};
    Info info {};
    QSharedPointer<Sql> sql {};
    QSharedPointer<SearchSql> search_sql {};
};

struct DataCenter {
    QHash<Section, const Info*> info_hash {};
    QHash<Section, AbstractTreeModel*> tree_model_hash {};
    QHash<Section, QSharedPointer<Sql>> sql_hash {};
    QHash<Section, const SectionRule*> section_rule_hash {};
    QHash<Section, const QHash<int, QString>*> leaf_path_hash {};
};

#endif // DATA_H
