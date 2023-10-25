#ifndef SEARCHTABLEMODEL_H
#define SEARCHTABLEMODEL_H

#include <QAbstractItemModel>

#include "component/info.h"
#include "component/using.h"
#include "sql/searchsql.h"

class SearchTableModel final : public QAbstractItemModel {
    Q_OBJECT
public:
    SearchTableModel(const Info* info, QSharedPointer<SearchSql> sql, QObject* parent = nullptr);
    ~SearchTableModel();

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
    QSharedPointer<SearchSql> sql_ {};

    SPTransactionList transaction_list_ {};
    const Info* info_ {};
};

#endif // SEARCHTABLEMODEL_H
