#include "transportmodel.h"

TransportModel::TransportModel(CStringList* table_header, const QHash<Section, QSharedPointer<Sql>>* table_sql_hash, QObject* parent)
    : QAbstractItemModel { parent }
    , table_header_ { table_header }
    , table_sql_hash_ { table_sql_hash }
{
}

TransportModel::~TransportModel() { transaction_list_.clear(); }

QModelIndex TransportModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex TransportModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int TransportModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return transaction_list_.size();
}

int TransportModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return table_header_->size();
}

QVariant TransportModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto transaction { transaction_list_.at(index.row()) };
    const TableColumn kColumn { index.column() };

    switch (kColumn) {
    case TableColumn::kID:
        return transaction->id;
    case TableColumn::kDateTime:
        return transaction->date_time;
    case TableColumn::kCode:
        return transaction->code;
    case TableColumn::kLhsNode:
        return transaction->lhs_node;
    case TableColumn::kLhsRatio:
        return transaction->lhs_ratio;
    case TableColumn::kLhsDebit:
        return transaction->lhs_debit == 0 ? QVariant() : transaction->lhs_debit;
    case TableColumn::kLhsCredit:
        return transaction->lhs_credit == 0 ? QVariant() : transaction->lhs_credit;
    case TableColumn::kDescription:
        return transaction->description;
    case TableColumn::kRhsNode:
        return transaction->rhs_node;
    case TableColumn::kTransport:
        return transaction->transport == 0 ? QVariant() : transaction->transport == 1 ? tr("S %1").arg(transaction->location.size() / 2) : tr("R");
    case TableColumn::kRhsRatio:
        return transaction->rhs_ratio;
    case TableColumn::kRhsDebit:
        return transaction->rhs_debit == 0 ? QVariant() : transaction->rhs_debit;
    case TableColumn::kRhsCredit:
        return transaction->rhs_credit == 0 ? QVariant() : transaction->rhs_credit;
    case TableColumn::kState:
        return transaction->state;
    case TableColumn::kDocument:
        return transaction->document.isEmpty() ? QVariant() : QString::number(transaction->document.size());
    default:
        return QVariant();
    }
}

QVariant TransportModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return table_header_->at(section);

    return QVariant();
}

void TransportModel::Query(CStringList& list)
{
    transaction_list_.clear();

    SPTransaction transaction {};

    beginResetModel();
    for (int i = 0; i != list.size(); ++i) {
        if (i % 2 == 1) {
            transaction = table_sql_hash_->value(Section { list.at(i - 1).toInt() })->Transaction(list.at(i).toInt());
            transaction_list_.emplaceBack(transaction);
        }
    }
    endResetModel();
}

void TransportModel::Add(CStringList& list)
{
    beginInsertRows(QModelIndex(), list.size(), list.size());
    auto transaction { table_sql_hash_->value(Section { list.at(0).toInt() })->Transaction(list.at(1).toInt()) };
    transaction_list_.emplaceBack(transaction);
    endInsertRows();
}

void TransportModel::Remove(int row)
{
    beginRemoveRows(QModelIndex(), row, row);
    transaction_list_.remove(row);
    endRemoveRows();
}
