#ifndef SEARCHTRANSMODEL_H
#define SEARCHTRANSMODEL_H

#include <QAbstractItemModel>

#include "component/using.h"
#include "sql/searchsql.h"
#include "table/tableinfo.h"

class SearchTransModel : public QAbstractItemModel {
    Q_OBJECT
public:
    SearchTransModel(const TableInfo* info, SearchSql* sql, QObject* parent = nullptr);
    ~SearchTransModel();

    SearchTransModel() = delete;
    SearchTransModel(const SearchTransModel&) = delete;
    SearchTransModel(SearchTransModel&&) = delete;
    SearchTransModel& operator=(const SearchTransModel&) = delete;
    SearchTransModel& operator=(SearchTransModel&&) = delete;

public:
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void sort(int column, Qt::SortOrder order) override;

public:
    void Query(const QString& text);

private:
    SearchSql* sql_ {};

    SPTransactionList trans_list_ {};
    const TableInfo* info_ {};
};

#endif // SEARCHTRANSMODEL_H
