#include "searchnodemodel.h"

#include "component/enumclass.h"

SearchNodeModel::SearchNodeModel(const TreeInfo* info, const TreeModel* tree_model, const SectionRule* section_rule, SearchSql* sql, QObject* parent)
    : QAbstractItemModel { parent }
    , sql_ { sql }
    , info_ { info }
    , section_rule_ { section_rule }
    , tree_model_ { tree_model }
{
}

QModelIndex SearchNodeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex SearchNodeModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int SearchNodeModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return node_list_.size();
}

int SearchNodeModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return info_->header.size();
}

QVariant SearchNodeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto node { node_list_.at(index.row()) };
    const NodeColumn kColumn { index.column() };

    switch (kColumn) {
    case NodeColumn::kName:
        return node->name;
    case NodeColumn::kID:
        return node->id;
    case NodeColumn::kCode:
        return node->code;
    case NodeColumn::kDescription:
        return node->description;
    case NodeColumn::kNote:
        return node->note;
    case NodeColumn::kNodeRule:
        return node->node_rule;
    case NodeColumn::kBranch:
        return node->branch;
    case NodeColumn::kUnit:
        return node->unit;
    case NodeColumn::kTotal:
        return node->unit == section_rule_->base_unit ? node->base_total : node->foreign_total;
    default:
        return QVariant();
    }
}

QVariant SearchNodeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return info_->header.at(section);

    return QVariant();
}

void SearchNodeModel::sort(int column, Qt::SortOrder order)
{
    if (column <= -1 || column >= info_->header.size())
        return;

    auto Compare = [column, order](const Node* lhs, const Node* rhs) -> bool {
        const NodeColumn kColumn { column };

        switch (kColumn) {
        case NodeColumn::kName:
            return (order == Qt::AscendingOrder) ? (lhs->name < rhs->name) : (lhs->name > rhs->name);
        case NodeColumn::kCode:
            return (order == Qt::AscendingOrder) ? (lhs->code < rhs->code) : (lhs->code > rhs->code);
        case NodeColumn::kDescription:
            return (order == Qt::AscendingOrder) ? (lhs->description < rhs->description) : (lhs->description > rhs->description);
        case NodeColumn::kNote:
            return (order == Qt::AscendingOrder) ? (lhs->note < rhs->note) : (lhs->note > rhs->note);
        case NodeColumn::kNodeRule:
            return (order == Qt::AscendingOrder) ? (lhs->node_rule < rhs->node_rule) : (lhs->node_rule > rhs->node_rule);
        case NodeColumn::kBranch:
            return (order == Qt::AscendingOrder) ? (lhs->branch < rhs->branch) : (lhs->branch > rhs->branch);
        case NodeColumn::kUnit:
            return (order == Qt::AscendingOrder) ? (lhs->unit < rhs->unit) : (lhs->unit > rhs->unit);
        case NodeColumn::kTotal:
            return (order == Qt::AscendingOrder) ? (lhs->base_total < rhs->base_total) : (lhs->base_total > rhs->base_total);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    std::sort(node_list_.begin(), node_list_.end(), Compare);
    emit layoutChanged();
}

void SearchNodeModel::Query(const QString& text)
{
    node_list_.clear();

    auto list { sql_->Node(text) };

    auto node_hash { tree_model_->GetNodeHash() };
    const Node* node {};

    beginResetModel();
    for (auto node_id : list) {
        node = node_hash->value(node_id, nullptr);

        if (node)
            node_list_.emplaceBack(node);
    }
    endResetModel();
}
