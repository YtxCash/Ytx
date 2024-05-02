#ifndef SEARCHTREEMODEL_H
#define SEARCHTREEMODEL_H

#include <QAbstractItemModel>

#include "component/info.h"
#include "component/settings.h"
#include "component/using.h"
#include "sql/searchsql.h"
#include "tree/treemodel.h"

class SearchTreeModel : public QAbstractItemModel {
    Q_OBJECT
public:
    SearchTreeModel(const Info* info, const TreeModel* tree_model, const SectionRule* section_rule, SearchSql* sql, QObject* parent = nullptr);
    ~SearchTreeModel() = default;

public:
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void sort(int column, Qt::SortOrder order) override;

public:
    void Query(CString& text);

private:
    SearchSql* sql_ {};

    const Info* info_ {};
    const SectionRule* section_rule_ {};
    const TreeModel* tree_model_ {};

    QList<const Node*> node_list_ {};
};

#endif // SEARCHTREEMODEL_H
