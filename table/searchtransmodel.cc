#include "searchtransmodel.h"

#include "component/enumclass.h"
#include "global/transactionpool.h"

SearchTransModel::SearchTransModel(const TableInfo* info, SearchSql* sql, QObject* parent)
    : QAbstractItemModel { parent }
    , sql_ { sql }
    , info_ { info }
{
}

SearchTransModel::~SearchTransModel() { trans_list_.clear(); }

QModelIndex SearchTransModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex SearchTransModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int SearchTransModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return trans_list_.size();
}

int SearchTransModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return info_->search_header.size();
}

QVariant SearchTransModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto trans { trans_list_.at(index.row()) };
    const SearchTransactionColumn kColumn { index.column() };

    switch (kColumn) {
    case SearchTransactionColumn::kID:
        return trans->id;
    case SearchTransactionColumn::kPostDate:
        return trans->date_time;
    case SearchTransactionColumn::kCode:
        return trans->code;
    case SearchTransactionColumn::kLhsNode:
        return trans->lhs_node;
    case SearchTransactionColumn::kLhsRatio:
        return trans->lhs_ratio;
    case SearchTransactionColumn::kLhsDebit:
        return trans->lhs_debit == 0 ? QVariant() : trans->lhs_debit;
    case SearchTransactionColumn::kLhsCredit:
        return trans->lhs_credit == 0 ? QVariant() : trans->lhs_credit;
    case SearchTransactionColumn::kDescription:
        return trans->description;
    case SearchTransactionColumn::kRhsNode:
        return trans->rhs_node;
    case SearchTransactionColumn::kRhsRatio:
        return trans->rhs_ratio;
    case SearchTransactionColumn::kRhsDebit:
        return trans->rhs_debit == 0 ? QVariant() : trans->rhs_debit;
    case SearchTransactionColumn::kRhsCredit:
        return trans->rhs_credit == 0 ? QVariant() : trans->rhs_credit;
    case SearchTransactionColumn::kState:
        return trans->state;
    case SearchTransactionColumn::kDocument:
        return trans->document.size() == 0 ? QVariant() : QString::number(trans->document.size());
    default:
        return QVariant();
    }
}

QVariant SearchTransModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return info_->search_header.at(section);

    return QVariant();
}

void SearchTransModel::sort(int column, Qt::SortOrder order)
{
    if (column <= -1 || column >= info_->search_header.size() - 1)
        return;

    auto Compare = [column, order](const SPTransaction& lhs, const SPTransaction& rhs) -> bool {
        const SearchTransactionColumn kColumn { column };

        switch (kColumn) {
        case SearchTransactionColumn::kPostDate:
            return (order == Qt::AscendingOrder) ? (lhs->date_time < rhs->date_time) : (lhs->date_time > rhs->date_time);
        case SearchTransactionColumn::kCode:
            return (order == Qt::AscendingOrder) ? (lhs->code < rhs->code) : (lhs->code > rhs->code);
        case SearchTransactionColumn::kLhsNode:
            return (order == Qt::AscendingOrder) ? (lhs->lhs_node < rhs->lhs_node) : (lhs->lhs_node > rhs->lhs_node);
        case SearchTransactionColumn::kLhsRatio:
            return (order == Qt::AscendingOrder) ? (lhs->lhs_ratio < rhs->lhs_ratio) : (lhs->lhs_ratio > rhs->lhs_ratio);
        case SearchTransactionColumn::kLhsDebit:
            return (order == Qt::AscendingOrder) ? (lhs->lhs_debit < rhs->lhs_debit) : (lhs->lhs_debit > rhs->lhs_debit);
        case SearchTransactionColumn::kLhsCredit:
            return (order == Qt::AscendingOrder) ? (lhs->lhs_credit < rhs->lhs_credit) : (lhs->lhs_credit > rhs->lhs_credit);
        case SearchTransactionColumn::kDescription:
            return (order == Qt::AscendingOrder) ? (lhs->description < rhs->description) : (lhs->description > rhs->description);
        case SearchTransactionColumn::kRhsNode:
            return (order == Qt::AscendingOrder) ? (lhs->rhs_node < rhs->rhs_node) : (lhs->rhs_node > rhs->rhs_node);
        case SearchTransactionColumn::kRhsRatio:
            return (order == Qt::AscendingOrder) ? (lhs->rhs_ratio < rhs->rhs_ratio) : (lhs->rhs_ratio > rhs->rhs_ratio);
        case SearchTransactionColumn::kRhsDebit:
            return (order == Qt::AscendingOrder) ? (lhs->rhs_debit < rhs->rhs_debit) : (lhs->rhs_debit > rhs->rhs_debit);
        case SearchTransactionColumn::kRhsCredit:
            return (order == Qt::AscendingOrder) ? (lhs->rhs_credit < rhs->rhs_credit) : (lhs->rhs_credit > rhs->rhs_credit);
        case SearchTransactionColumn::kState:
            return (order == Qt::AscendingOrder) ? (lhs->state < rhs->state) : (lhs->state > rhs->state);
        case SearchTransactionColumn::kDocument:
            return (order == Qt::AscendingOrder) ? (lhs->document.size() < rhs->document.size()) : (lhs->document.size() > rhs->document.size());
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(trans_list_.begin(), trans_list_.end(), Compare);
    emit layoutChanged();
}

void SearchTransModel::Query(const QString& text)
{
    if (!trans_list_.isEmpty())
        trans_list_.clear();

    SPTransactionList list {};
    if (!text.isEmpty())
        list = sql_->Trans(text);

    beginResetModel();
    for (auto& trans : list)
        trans_list_.emplace_back(trans);

    endResetModel();
}
