#ifndef TRANSPORTMODEL_H
#define TRANSPORTMODEL_H

#include <QAbstractItemModel>

#include "component/using.h"
#include "sql/tablesql.h"

class TransportModel : public QAbstractItemModel {
    Q_OBJECT
public:
    TransportModel(CStringList* table_header, const QHash<Section, TableSql*>* table_sql_hash, QObject* parent = nullptr);
    ~TransportModel();

public:
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void Query(CStringList& list);
    void Add(CStringList& list);
    void Remove(int row);

private:
    void IniHash();

private:
    CStringList* table_header_ {};
    const QHash<Section, TableSql*>* table_sql_hash_ {};

    SPTransactionList transaction_list_ {};
};

#endif // TRANSPORTMODEL_H
