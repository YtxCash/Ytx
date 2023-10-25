#include "tablemodelstakeholder.h"

#include "component/constvalue.h"
#include "global/transpool.h"

TableModelStakeholder::TableModelStakeholder(const Info* info, const SectionRule* section_rule, QSharedPointer<Sql> sql, int node_id, int mark, QObject* parent)
    : AbstractTableModel { parent }
    , info_ { info }
    , section_rule_ { section_rule }
    , sql_ { sql }
    , node_id_ { node_id }
    , mark_ { mark }
{
    trans_list_ = sql_->TransList(node_id);
}

TableModelStakeholder::~TableModelStakeholder() { RecycleTrans(trans_list_); }

bool TableModelStakeholder::AppendOne(const QModelIndex& parent)
{
    // just register trans this function
    // while set related node in setData function, register related transaction to sql_'s transaction_hash_
    auto row { trans_list_.size() };
    auto trans { sql_->AllocateTrans() };

    *trans->node = node_id_;

    beginInsertRows(parent, row, row);
    trans_list_.emplaceBack(trans);
    endInsertRows();

    return true;
}

bool TableModelStakeholder::DeleteOne(int row, const QModelIndex& parent)
{
    if (row <= -1)
        return false;

    auto trans { trans_list_.at(row) };
    int related_node_id { *trans->related_node };

    beginRemoveRows(parent, row, row);
    trans_list_.removeAt(row);
    endRemoveRows();

    if (related_node_id != 0)
        sql_->Delete(*trans->id);

    TransPool::Instance().Recycle(trans);
    return true;
}

void TableModelStakeholder::RMoveMulti(int old_node_id, int new_node_id, const QList<int>& trans_id_list)
{
    if (node_id_ == old_node_id)
        RemoveMulti(trans_id_list);

    if (node_id_ == new_node_id)
        AppendMulti(new_node_id, trans_id_list);
}

bool TableModelStakeholder::RemoveMulti(const QList<int>& trans_id_list)
{
    int min_row {};
    int trans_id {};

    for (int i = 0; i != trans_list_.size(); ++i) {
        trans_id = *trans_list_.at(i)->id;

        if (trans_id_list.contains(trans_id)) {
            beginRemoveRows(QModelIndex(), i, i);
            TransPool::Instance().Recycle(trans_list_.takeAt(i));
            endRemoveRows();

            if (min_row == 0)
                min_row = i;

            --i;
        }
    }

    return true;
}

QModelIndex TableModelStakeholder::TransIndex(int trans_id) const
{
    int row { 0 };

    for (const auto& trans : trans_list_) {
        if (*trans->id == trans_id) {
            return index(row, 0);
        }
        ++row;
    }
    return QModelIndex();
}

int TableModelStakeholder::NodeRow(int node_id) const
{
    int row { 0 };

    for (const auto& trans : trans_list_) {
        if (*trans->related_node == node_id) {
            return row;
        }
        ++row;
    }
    return -1;
}

void TableModelStakeholder::RecycleTrans(SPTransList& list)
{
    for (auto& trans : list)
        TransPool::Instance().Recycle(trans);

    list.clear();
}

QModelIndex TableModelStakeholder::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex TableModelStakeholder::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int TableModelStakeholder::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return trans_list_.size();
}

int TableModelStakeholder::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return info_->part_table_header.size();
}

QVariant TableModelStakeholder::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto trans { trans_list_.at(index.row()) };
    const PartTableColumn kColumn { index.column() };

    switch (kColumn) {
    case PartTableColumn::kID:
        return *trans->id;
    case PartTableColumn::kDateTime:
        return *trans->date_time;
    case PartTableColumn::kCode:
        return *trans->code;
    case PartTableColumn::kRatio:
        return mark_ == 0 ? QVariant() : *trans->ratio;
    case PartTableColumn::kDescription:
        return *trans->description;
    case PartTableColumn::kDebit:
        return mark_ == 0 ? *trans->related_debit : QVariant();
    case PartTableColumn::kRelatedNode:
        return *trans->related_node == 0 ? QVariant() : *trans->related_node;
    case PartTableColumn::kDocument:
        return trans->document->isEmpty() ? QVariant() : QString::number(trans->document->size());
    default:
        return QVariant();
    }
}

bool TableModelStakeholder::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    const PartTableColumn kColumn { index.column() };
    const int kRow { index.row() };

    auto trans { trans_list_.at(kRow) };
    int old_related_node { *trans->related_node };

    bool rel_changed { false };

    switch (kColumn) {
    case PartTableColumn::kDateTime:
        UpdateDateTime(trans, value.toString());
        break;
    case PartTableColumn::kCode:
        UpdateCode(trans, value.toString());
        break;
    case PartTableColumn::kDescription:
        UpdateDescription(trans, value.toString());
        break;
    case PartTableColumn::kRatio:
        UpdateUnitPrice(trans, value.toDouble());
        break;
    case PartTableColumn::kDebit:
        UpdateCommission(trans, value.toDouble());
        break;
    case PartTableColumn::kRelatedNode:
        rel_changed = UpdateRelatedNode(trans, value.toInt());
        break;
    default:
        return false;
    }

    if (old_related_node == 0) {
        if (rel_changed) {
            sql_->Insert(trans);
        }

        emit SResizeColumnToContents(index.column());
        return true;
    }

    if (rel_changed)
        sql_->Update(*trans->id);

    emit SResizeColumnToContents(index.column());
    return true;
}

bool TableModelStakeholder::UpdateDateTime(SPTrans& trans, CString& value)
{
    if (*trans->date_time == value)
        return false;

    *trans->date_time = value;

    if (*trans->related_node != 0)
        sql_->Update(info_->transaction, DATE_TIME, value, *trans->id);

    return true;
}

bool TableModelStakeholder::UpdateDescription(SPTrans& trans, CString& value)
{
    if (*trans->description == value)
        return false;

    *trans->description = value;

    if (*trans->related_node != 0) {
        sql_->Update(info_->transaction, DESCRIPTION, value, *trans->id);
        emit SSearch();
    }

    return true;
}

bool TableModelStakeholder::UpdateCode(SPTrans& trans, CString& value)
{
    if (*trans->code == value)
        return false;

    *trans->code = value;

    if (*trans->related_node != 0)
        sql_->Update(info_->transaction, CODE, value, *trans->id);

    return true;
}

bool TableModelStakeholder::UpdateUnitPrice(SPTrans& trans, double value)
{
    const double tolerance { std::pow(10, -section_rule_->ratio_decimal - 2) };

    if (std::abs(*trans->ratio - value) < tolerance || mark_ == 0)
        return false;

    *trans->ratio = value;

    if (*trans->related_node != 0)
        sql_->Update(info_->transaction, UNIT_PRICE, value, *trans->id);

    return true;
}

bool TableModelStakeholder::UpdateCommission(SPTrans& trans, double value)
{
    const double tolerance { std::pow(10, -section_rule_->ratio_decimal - 2) };

    if (std::abs(*trans->related_debit - value) < tolerance || mark_ != 0)
        return false;

    *trans->related_debit = value;

    if (*trans->related_node != 0)
        sql_->Update(info_->transaction, COMMISSION, value, *trans->id);

    return true;
}

bool TableModelStakeholder::UpdateRelatedNode(SPTrans& trans, int value)
{
    if (*trans->related_node == value)
        return false;

    *trans->related_node = value;
    return true;
}

QVariant TableModelStakeholder::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return info_->part_table_header.at(section);

    return QVariant();
}

void TableModelStakeholder::sort(int column, Qt::SortOrder order)
{
    // ignore balance column
    if (column <= -1 || column >= info_->part_table_header.size() - 1)
        return;

    auto Compare = [column, order](SPTrans& lhs, SPTrans& rhs) -> bool {
        const PartTableColumn kColumn { column };

        switch (kColumn) {
        case PartTableColumn::kDateTime:
            return (order == Qt::AscendingOrder) ? (*lhs->date_time < *rhs->date_time) : (*lhs->date_time > *rhs->date_time);
        case PartTableColumn::kCode:
            return (order == Qt::AscendingOrder) ? (*lhs->code < *rhs->code) : (*lhs->code > *rhs->code);
        case PartTableColumn::kRatio:
            return (order == Qt::AscendingOrder) ? (*lhs->ratio < *rhs->ratio) : (*lhs->ratio > *rhs->ratio);
        case PartTableColumn::kDescription:
            return (order == Qt::AscendingOrder) ? (*lhs->description < *rhs->description) : (*lhs->description > *rhs->description);
        case PartTableColumn::kRelatedNode:
            return (order == Qt::AscendingOrder) ? (*lhs->related_node < *rhs->related_node) : (*lhs->related_node > *rhs->related_node);
        case PartTableColumn::kDocument:
            return (order == Qt::AscendingOrder) ? (lhs->document->size() < rhs->document->size()) : (lhs->document->size() > rhs->document->size());
        case PartTableColumn::kTransport:
            return (order == Qt::AscendingOrder) ? (lhs->transport < rhs->transport) : (lhs->transport > rhs->transport);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(trans_list_.begin(), trans_list_.end(), Compare);
    emit layoutChanged();
}

bool TableModelStakeholder::AppendMulti(int node_id, const QList<int>& trans_id_list)
{
    auto row { trans_list_.size() };
    auto trans_list { sql_->TransList(node_id, trans_id_list) };

    beginInsertRows(QModelIndex(), row, row + trans_list.size() - 1);
    trans_list_.append(trans_list);
    endInsertRows();

    return true;
}

Qt::ItemFlags TableModelStakeholder::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    auto flags { QAbstractItemModel::flags(index) };
    const PartTableColumn kColumn { index.column() };

    switch (kColumn) {
    case PartTableColumn::kID:
    case PartTableColumn::kDocument:
        flags &= ~Qt::ItemIsEditable;
        break;
    default:
        flags |= Qt::ItemIsEditable;
    }

    return flags;
}
