#include "treemodelorder.h"

#include <QMimeData>
#include <QQueue>
#include <QRegularExpression>

#include "component/constvalue.h"
#include "component/enumclass.h"
#include "global/nodepool.h"

TreeModelOrder::TreeModelOrder(
    const Info* info, QSharedPointer<Sql> sql, const SectionRule* section_rule, const TableHash* table_hash, const Interface* interface, QObject* parent)
    : AbstractTreeModel { parent }
    , info_ { info }
    , section_rule_ { section_rule }
    , table_hash_ { table_hash }
    , interface_ { interface }
    , sql_ { sql }
{
    root_ = NodePool::Instance().Allocate();
    root_->id = -1;
    root_->branch = true;
    root_->unit = section_rule->base_unit;

    IniTree(node_hash_);
}

TreeModelOrder::~TreeModelOrder() { RecycleNode(node_hash_); }

bool TreeModelOrder::RUpdateMultiTotal(const QList<int>& node_list)
{
    double old_final_total {};
    double old_initial_total {};
    double final_diff {};
    double initial_diff {};
    Node* node {};

    for (const auto& node_id : node_list) {
        node = const_cast<Node*>(GetNode(node_id));

        if (!node || node->branch)
            continue;

        old_final_total = node->final_total;
        old_initial_total = node->initial_total;

        sql_->LeafTotal(node);
        UpdateLeafTotal(node);

        final_diff = node->final_total - old_final_total;
        initial_diff = node->initial_total - old_initial_total;

        // UpdateBranchTotal(node, final_diff, initial_diff);
    }

    emit SUpdateDSpinBox();
    return true;
}

void TreeModelOrder::UpdateNode(const Node* tmp_node)
{
    if (!tmp_node)
        return;

    auto node { node_hash_.value(tmp_node->id) };

    if (*node == *tmp_node)
        return;

    UpdateFirstProperty(node, tmp_node->first_property);
    UpdateEmployee(node, tmp_node->second_property);
    UpdateThirdProperty(node, tmp_node->third_property);
    UpdateDiscount(node, tmp_node->fourth_property);
    UpdateRefund(node, tmp_node->fifth_property);
    UpdateDateTime(node, tmp_node->date_time);
    UpdateDescription(node, tmp_node->description);
    UpdatePosted(node, tmp_node->node_rule);
    UpdateBranch(node, tmp_node->branch);
    UpdateTerm(node, tmp_node->unit);

    if (node->name != tmp_node->name) {
        UpdateName(node, tmp_node->name);
        emit SUpdateName(node);
    }
}

bool TreeModelOrder::RRemoveNode(int node_id)
{
    auto index { GetIndex(node_id) };
    auto row { index.row() };
    auto parent_index { index.parent() };
    RemoveRow(row, parent_index);

    return true;
}

void TreeModelOrder::RUpdateOneTotal(int node_id, double final_debit_diff, double final_credit_diff, double initial_debit_diff, double initial_credit_diff)
{
    auto node { const_cast<Node*>(GetNode(node_id)) };
    auto node_rule { node->node_rule };

    auto final_diff { node_rule ? final_credit_diff - final_debit_diff : final_debit_diff - final_credit_diff };
    auto initial_diff { node_rule ? initial_credit_diff - initial_debit_diff : initial_debit_diff - initial_credit_diff };

    node->final_total += final_diff;
    node->initial_total += initial_diff;

    UpdateLeafTotal(node);
    // UpdateBranchTotal(node, final_diff, initial_diff);
    emit SUpdateDSpinBox();
}

void TreeModelOrder::RUpdateProperty(int node_id, double first, double third, double fourth) { }

void TreeModelOrder::IniTree(NodeHash& node_hash)
{
    sql_->Tree(node_hash);

    for (auto& node : std::as_const(node_hash)) {
        if (!node->parent) {
            node->parent = root_;
            root_->children.emplace_back(node);
        }
    }

    for (auto& node : std::as_const(node_hash))
        UpdateBranchTotal(node, node->first_property, node->third_property, node->initial_total, node->final_total);

    node_hash.insert(-1, root_);
}

void TreeModelOrder::UpdateBranchTotal(const Node* node, double primary_diff, double secondary_diff, double initial_diff, double final_diff)
{
    if (!node)
        return;

    while (node != root_) {
        node->parent->initial_total += initial_diff;
        node->parent->first_property += primary_diff;
        node->parent->third_property += secondary_diff;
        node->parent->final_total += final_diff;
        node->parent->note = QString::number(node->note.toDouble() + initial_diff - final_diff);
        node = node->parent;
    }
}

QString TreeModelOrder::CreatePath(const Node* node) const
{
    if (!node || node == root_)
        return QString();

    QStringList tmp {};

    while (node != root_) {
        tmp.prepend(node->name);
        node = node->parent;
    }

    return tmp.join(interface_->separator);
}

void TreeModelOrder::UpdateBranchUnit(Node* node) const
{
    if (!node || !node->branch || node->unit == section_rule_->base_unit)
        return;

    QQueue<const Node*> queue {};
    queue.enqueue(node);

    const Node* queue_node {};
    double initial_total {};
    bool equal {};
    bool branch {};

    int unit { node->unit };
    bool node_rule { node->node_rule };

    while (!queue.isEmpty()) {
        queue_node = queue.dequeue();
        branch = queue_node->branch;

        if (branch)
            for (const auto& child : queue_node->children)
                queue.enqueue(child);

        if (!branch && queue_node->unit == unit) {
            equal = queue_node->node_rule == node_rule;
            initial_total += equal ? queue_node->initial_total : -queue_node->initial_total;
        }
    }

    node->initial_total = initial_total;
}

void TreeModelOrder::RecycleNode(NodeHash& node_hash)
{
    for (auto& node : std::as_const(node_hash))
        NodePool::Instance().Recycle(node);

    node_hash.clear();
}

QModelIndex TreeModelOrder::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    auto parent_node { GetNode(parent) };
    auto node { parent_node->children.at(row) };

    return node ? createIndex(row, column, node) : QModelIndex();
}

QModelIndex TreeModelOrder::parent(const QModelIndex& index) const
{
    // root_'s index is QModelIndex();

    if (!index.isValid())
        return QModelIndex();

    auto node { GetNode(index) };
    if (node == root_)
        return QModelIndex();

    auto parent_node { node->parent };
    if (parent_node == root_)
        return QModelIndex();

    return createIndex(parent_node->parent->children.indexOf(parent_node), 0, parent_node);
}

int TreeModelOrder::rowCount(const QModelIndex& parent) const { return GetNode(parent)->children.size(); }

bool TreeModelOrder::UpdateBranch(Node* node, bool value)
{
    int node_id { node->id };
    if (node->branch == value || !node->children.isEmpty() || sql_->InternalReferences(node_id) || table_hash_->contains(node_id))
        return false;

    node->branch = value;
    sql_->Update(info_->node, BRANCH, value, node_id);

    return true;
}

bool TreeModelOrder::UpdateDescription(Node* node, CString& value)
{
    if (node->description == value)
        return false;

    node->description = value;
    sql_->Update(info_->node, DESCRIPTION, value, node->id);
    return true;
}

bool TreeModelOrder::UpdateTerm(Node* node, int value)
{
    int node_id { node->id };
    if (node->unit == value)
        return false;

    node->unit = value;
    sql_->Update(info_->node, UNIT, value, node_id);

    if (node->branch)
        UpdateBranchUnit(node);

    return true;
}

bool TreeModelOrder::UpdateName(Node* node, CString& value)
{
    node->name = value;
    sql_->Update(info_->node, NAME, value, node->id);

    emit SResizeColumnToContents(std::to_underlying(TreeColumn::kName));
    emit SSearch();
    return true;
}

bool TreeModelOrder::UpdateFirstProperty(Node* node, int value)
{
    if (node->first_property == value)
        return false;

    node->first_property = value;
    sql_->Update(info_->node, FIRST_PROPERTY, value, node->id);
    return true;
}

bool TreeModelOrder::UpdateEmployee(Node* node, int value)
{
    if (node->second_property == value)
        return false;

    node->second_property = value;
    sql_->Update(info_->node, EMPLOYEE, value, node->id);
    return true;
}

bool TreeModelOrder::UpdateThirdProperty(Node* node, double value)
{
    const double tolerance { std::pow(10, -section_rule_->ratio_decimal - 2) };

    if (std::abs(node->third_property - value) < tolerance)
        return false;

    node->third_property = value;
    sql_->Update(info_->node, THIRD_PROPERTY, value, node->id);

    return true;
}

bool TreeModelOrder::UpdateDiscount(Node* node, double value)
{
    const double tolerance { std::pow(10, -section_rule_->ratio_decimal - 2) };

    if (std::abs(node->fourth_property - value) < tolerance)
        return false;

    node->fourth_property = value;
    sql_->Update(info_->node, DISCOUNT, value, node->id);

    return true;
}

bool TreeModelOrder::UpdateRefund(Node* node, bool value)
{
    if (node->fifth_property == value)
        return false;

    node->fifth_property = value;
    sql_->Update(info_->node, REFUND, value, node->id);
    return true;
}

bool TreeModelOrder::UpdateDateTime(Node* node, CString& value)
{
    if (node->date_time == value)
        return false;

    node->date_time = value;
    sql_->Update(info_->node, DATE_TIME, value, node->id);
    return true;
}

bool TreeModelOrder::UpdatePosted(Node* node, bool value)
{
    if (node->node_rule == value)
        return false;

    node->node_rule = value;
    sql_->Update(info_->node, NODE_RULE, value, node->id);

    node->final_total = -node->final_total;
    node->initial_total = -node->initial_total;
    if (!node->branch)
        emit SNodeRule(node->id, value);

    return true;
}

void TreeModelOrder::sort(int column, Qt::SortOrder order)
{
    if (column <= -1 || column >= info_->tree_header.size())
        return;

    auto Compare = [column, order](const Node* lhs, const Node* rhs) -> bool {
        const TreeColumn kColumn { column };
        switch (kColumn) {
        case TreeColumn::kName:
            return (order == Qt::AscendingOrder) ? (lhs->name < rhs->name) : (lhs->name > rhs->name);
        case TreeColumn::kDescription:
            return (order == Qt::AscendingOrder) ? (lhs->description < rhs->description) : (lhs->description > rhs->description);
        case TreeColumn::kNodeRule:
            return (order == Qt::AscendingOrder) ? (lhs->node_rule < rhs->node_rule) : (lhs->node_rule > rhs->node_rule);
        case TreeColumn::kBranch:
            return (order == Qt::AscendingOrder) ? (lhs->branch < rhs->branch) : (lhs->branch > rhs->branch);
        case TreeColumn::kUnit:
            return (order == Qt::AscendingOrder) ? (lhs->unit < rhs->unit) : (lhs->unit > rhs->unit);
        case TreeColumn::kThird:
            return (order == Qt::AscendingOrder) ? (lhs->third_property < rhs->third_property) : (lhs->third_property > rhs->third_property);
        case TreeColumn::kFirst:
            return (order == Qt::AscendingOrder) ? (lhs->first_property < rhs->first_property) : (lhs->first_property > rhs->first_property);
        case TreeColumn::kSecond:
            return (order == Qt::AscendingOrder) ? (lhs->second_property < rhs->second_property) : (lhs->second_property > rhs->second_property);
        case TreeColumn::kFourth:
            return (order == Qt::AscendingOrder) ? (lhs->fourth_property < rhs->fourth_property) : (lhs->fourth_property > rhs->fourth_property);
        case TreeColumn::kFifth:
            return (order == Qt::AscendingOrder) ? (lhs->fifth_property < rhs->fifth_property) : (lhs->fifth_property > rhs->fifth_property);
        case TreeColumn::kDateTime:
            return (order == Qt::AscendingOrder) ? (lhs->date_time < rhs->date_time) : (lhs->date_time > rhs->date_time);
        case TreeColumn::kInitialTotal:
            return (order == Qt::AscendingOrder) ? (lhs->initial_total < rhs->initial_total) : (lhs->initial_total > rhs->initial_total);
        case TreeColumn::kFinalTotal:
            return (order == Qt::AscendingOrder) ? (lhs->final_total < rhs->final_total) : (lhs->final_total > rhs->final_total);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    SortIterative(root_, Compare);
    emit layoutChanged();
}

void TreeModelOrder::SortIterative(Node* node, std::function<bool(const Node*, const Node*)> Compare)
{
    if (!node)
        return;

    QQueue<Node*> queue {};
    queue.enqueue(node);

    Node* queue_node {};

    while (!queue.isEmpty()) {
        queue_node = queue.dequeue();

        if (!queue_node->children.isEmpty()) {
            std::sort(queue_node->children.begin(), queue_node->children.end(), Compare);
            for (const auto& child : queue_node->children) {
                queue.enqueue(child);
            }
        }
    }
}

Node* TreeModelOrder::GetNode(const QModelIndex& index) const
{
    if (index.isValid() && index.internalPointer())
        return static_cast<Node*>(index.internalPointer());

    return root_;
}

QModelIndex TreeModelOrder::GetIndex(int node_id) const
{
    if (node_id == -1 || !node_hash_.contains(node_id))
        return QModelIndex();

    auto node { node_hash_.value(node_id) };
    return createIndex(node->parent->children.indexOf(node), 0, node);
}

bool TreeModelOrder::IsDescendant(Node* lhs, Node* rhs) const
{
    if (!lhs || !rhs || lhs == rhs)
        return false;

    while (lhs && lhs != rhs)
        lhs = lhs->parent;

    return lhs == rhs;
}

bool TreeModelOrder::InsertRow(int row, const QModelIndex& parent, Node* node)
{
    if (row <= -1)
        return false;

    auto parent_node { GetNode(parent) };

    beginInsertRows(parent, row, row);
    parent_node->children.insert(row, node);
    endInsertRows();

    sql_->Insert(parent_node->id, node);
    node_hash_.insert(node->id, node);

    emit SSearch();
    return true;
}

bool TreeModelOrder::RemoveRow(int row, const QModelIndex& parent)
{
    if (row <= -1 || row >= rowCount(parent))
        return false;

    auto parent_node { GetNode(parent) };
    auto node { parent_node->children.at(row) };

    int node_id { node->id };
    bool branch { node->branch };

    beginRemoveRows(parent, row, row);
    if (branch) {
        for (auto& child : node->children) {
            child->parent = parent_node;
            parent_node->children.emplace_back(child);
        }
    }
    parent_node->children.removeOne(node);
    endRemoveRows();

    if (branch) {
        sql_->Remove(node_id, true);
        emit SUpdateName(node);
    }

    if (!branch) {
        // UpdateBranchTotal(node, -node->final_total, -node->initial_total);
        sql_->Remove(node_id, false);
    }

    emit SSearch();
    emit SResizeColumnToContents(std::to_underlying(TreeColumn::kName));

    NodePool::Instance().Recycle(node);
    node_hash_.remove(node_id);

    return true;
}

bool TreeModelOrder::UpdateLeafTotal(const Node* node)
{
    if (!node || node->branch)
        return false;

    auto node_id { node->id };

    sql_->Update(info_->node, FINAL_TOTAL, node->final_total, node_id);
    sql_->Update(info_->node, INITIAL_TOTAL, node->initial_total, node_id);

    return true;
}

QVariant TreeModelOrder::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return info_->tree_header.at(section);

    return QVariant();
}

QVariant TreeModelOrder::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto node { GetNode(index) };
    if (node->id == -1)
        return QVariant();

    const TreeColumn kColumn { index.column() };

    switch (kColumn) {
    case TreeColumn::kName:
        return node->name;
    case TreeColumn::kID:
        return node->id;
    case TreeColumn::kFirst:
        return node->first_property;
    case TreeColumn::kSecond:
        return node->second_property;
    case TreeColumn::kThird:
        return node->third_property;
    case TreeColumn::kFourth:
        return node->fourth_property;
    case TreeColumn::kFifth:
        return node->fifth_property;
    case TreeColumn::kDateTime:
        return node->date_time;
    case TreeColumn::kDescription:
        return node->description;
    case TreeColumn::kNodeRule:
        return node->node_rule;
    case TreeColumn::kBranch:
        return node->branch;
    case TreeColumn::kUnit:
        return node->unit;
    case TreeColumn::kInitialTotal:
        return node->initial_total;
    case TreeColumn::kFinalTotal:
        return node->final_total;
    default:
        return QVariant();
    }
}

Qt::ItemFlags TreeModelOrder::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    auto flags { QAbstractItemModel::flags(index) };
    const TreeColumn kColumn { index.column() };

    switch (kColumn) {
    case TreeColumn::kName:
        flags |= Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
        flags &= ~Qt::ItemIsEditable;
        break;
    default:
        flags &= ~Qt::ItemIsEditable;
        break;
    }

    return flags;
}

QMimeData* TreeModelOrder::mimeData(const QModelIndexList& indexes) const
{
    auto mime_data { new QMimeData() };
    if (indexes.isEmpty())
        return mime_data;

    auto first_index = indexes.first();

    if (first_index.isValid()) {
        int id { first_index.sibling(first_index.row(), std::to_underlying(TreeColumn::kID)).data().toInt() };
        mime_data->setData(NODE_ID, QByteArray::number(id));
    }

    return mime_data;
}

bool TreeModelOrder::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex&) const
{
    Q_UNUSED(row);
    Q_UNUSED(column);

    return action != Qt::IgnoreAction && data && data->hasFormat(NODE_ID);
}

bool TreeModelOrder::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
    if (!canDropMimeData(data, action, row, column, parent))
        return false;

    auto destination_parent { GetNode(parent) };
    if (!destination_parent->branch)
        return false;

    int node_id {};

    if (auto mime { data->data(NODE_ID) }; !mime.isEmpty())
        node_id = QVariant(mime).toInt();

    auto node { const_cast<Node*>(GetNode(node_id)) };
    if (!node || node->parent == destination_parent || IsDescendant(destination_parent, node))
        return false;

    if (node->unit != destination_parent->unit) {
        node->unit = destination_parent->unit;
    }

    auto begin_row { row == -1 ? destination_parent->children.size() : row };
    auto source_row { node->parent->children.indexOf(node) };
    auto source_index { createIndex(node->parent->children.indexOf(node), 0, node) };

    if (beginMoveRows(source_index.parent(), source_row, source_row, parent, begin_row)) {
        node->parent->children.removeAt(source_row);
        // UpdateOrderBranchTotal(node, -node->first_property, -node->third_property, -node->initial_total, -node->final_total);

        destination_parent->children.insert(begin_row, node);
        node->parent = destination_parent;
        // UpdateOrderBranchTotal(node, node->first_property, node->third_property, node->initial_total, node->final_total);

        endMoveRows();
    }

    sql_->Drag(destination_parent->id, node_id);
    emit SResizeColumnToContents(std::to_underlying(TreeColumn::kName));
    emit SUpdateName(node);

    return true;
}
