#include "searchtreemodel.h"

#include "component/enumclass.h"

SearchTreeModel::SearchTreeModel(
    const Info* info, const AbstractTreeModel* tree_model, const SectionRule* section_rule, QSharedPointer<SearchSql> sql, QObject* parent)
    : QAbstractItemModel { parent }
    , sql_ { sql }
    , info_ { info }
    , section_rule_ { section_rule }
    , tree_model_ { tree_model }
{
}

QModelIndex SearchTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex SearchTreeModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int SearchTreeModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return node_list_.size();
}

int SearchTreeModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return info_->tree_header.size();
}

QVariant SearchTreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto node { node_list_.at(index.row()) };
    const TreeColumn kColumn { index.column() };

    switch (kColumn) {
    case TreeColumn::kName:
        return node->name;
    case TreeColumn::kID:
        return node->id;
    case TreeColumn::kCode:
        return node->code;
    case TreeColumn::kThird:
        return node->third_property;
    case TreeColumn::kDescription:
        return node->description;
    case TreeColumn::kNote:
        return node->note;
    case TreeColumn::kNodeRule:
        return node->node_rule;
    case TreeColumn::kBranch:
        return node->branch;
    case TreeColumn::kUnit:
        return node->unit;
    case TreeColumn::kInitialTotal:
        return node->unit == section_rule_->base_unit ? node->final_total : node->initial_total;
    default:
        return QVariant();
    }
}

QVariant SearchTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return info_->tree_header.at(section);

    return QVariant();
}

void SearchTreeModel::sort(int column, Qt::SortOrder order)
{
    if (column <= -1 || column >= info_->tree_header.size())
        return;

    auto Compare = [column, order](const Node* lhs, const Node* rhs) -> bool {
        const TreeColumn kColumn { column };

        switch (kColumn) {
        case TreeColumn::kName:
            return (order == Qt::AscendingOrder) ? (lhs->name < rhs->name) : (lhs->name > rhs->name);
        case TreeColumn::kCode:
            return (order == Qt::AscendingOrder) ? (lhs->code < rhs->code) : (lhs->code > rhs->code);
        case TreeColumn::kThird:
            return (order == Qt::AscendingOrder) ? (lhs->third_property < rhs->third_property) : (lhs->third_property > rhs->third_property);
        case TreeColumn::kDescription:
            return (order == Qt::AscendingOrder) ? (lhs->description < rhs->description) : (lhs->description > rhs->description);
        case TreeColumn::kNote:
            return (order == Qt::AscendingOrder) ? (lhs->note < rhs->note) : (lhs->note > rhs->note);
        case TreeColumn::kNodeRule:
            return (order == Qt::AscendingOrder) ? (lhs->node_rule < rhs->node_rule) : (lhs->node_rule > rhs->node_rule);
        case TreeColumn::kBranch:
            return (order == Qt::AscendingOrder) ? (lhs->branch < rhs->branch) : (lhs->branch > rhs->branch);
        case TreeColumn::kUnit:
            return (order == Qt::AscendingOrder) ? (lhs->unit < rhs->unit) : (lhs->unit > rhs->unit);
        case TreeColumn::kInitialTotal:
            return (order == Qt::AscendingOrder) ? (lhs->final_total < rhs->final_total) : (lhs->final_total > rhs->final_total);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(node_list_.begin(), node_list_.end(), Compare);
    emit layoutChanged();
}

void SearchTreeModel::Query(const QString& text)
{
    node_list_.clear();

    auto list { sql_->Node(text) };

    auto node_hash { tree_model_->GetNodeHash() };
    const Node* node {};

    beginResetModel();
    for (const auto& node_id : list) {
        node = node_hash->value(node_id, nullptr);

        if (node)
            node_list_.emplaceBack(node);
    }
    endResetModel();
}
