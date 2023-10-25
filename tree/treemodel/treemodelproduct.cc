#include "treemodelproduct.h"

#include <QMimeData>
#include <QQueue>
#include <QRegularExpression>

#include "component/constvalue.h"
#include "component/enumclass.h"
#include "global/nodepool.h"

TreeModelProduct::TreeModelProduct(
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

    IniTree(node_hash_, leaf_path_, branch_path_);
}

TreeModelProduct::~TreeModelProduct() { RecycleNode(node_hash_); }

bool TreeModelProduct::RUpdateMultiTotal(const QList<int>& node_list)
{
    double old_initial_total {};
    double initial_diff {};
    Node* node {};

    for (const auto& node_id : node_list) {
        node = const_cast<Node*>(GetNode(node_id));

        if (!node || node->branch)
            continue;

        old_initial_total = node->initial_total;

        sql_->LeafTotal(node);
        UpdateLeafTotal(node);

        initial_diff = node->initial_total - old_initial_total;
        UpdateBranchTotal(node, initial_diff);
    }

    emit SUpdateDSpinBox();
    return true;
}

void TreeModelProduct::UpdateNode(const Node* tmp_node)
{
    if (!tmp_node)
        return;

    auto node { const_cast<Node*>(GetNode(tmp_node->id)) };
    UpdateUnitPrice(node, tmp_node->third_property);

    if (*node == *tmp_node)
        return;

    UpdateBranch(node, tmp_node->branch);
    UpdateCode(node, tmp_node->code);
    UpdateDescription(node, tmp_node->description);
    UpdateNote(node, tmp_node->note);
    UpdateRule(node, tmp_node->node_rule);
    UpdateUnit(node, tmp_node->unit);

    if (node->name != tmp_node->name) {
        UpdateName(node, tmp_node->name);
        emit SUpdateName(node);
    }
}

void TreeModelProduct::UpdateSeparator(CString& separator)
{
    if (interface_->separator == separator || separator.isEmpty())
        return;

    for (auto& path : leaf_path_)
        path.replace(interface_->separator, separator);

    for (auto& path : branch_path_)
        path.replace(interface_->separator, separator);
}

bool TreeModelProduct::RRemoveNode(int node_id)
{
    auto index { GetIndex(node_id) };
    auto row { index.row() };
    auto parent_index { index.parent() };
    RemoveRow(row, parent_index);

    return true;
}

void TreeModelProduct::RUpdateOneTotal(int node_id, double final_debit_diff, double final_credit_diff, double initial_debit_diff, double initial_credit_diff)
{
    Q_UNUSED(final_debit_diff)
    Q_UNUSED(final_credit_diff)

    auto node { const_cast<Node*>(GetNode(node_id)) };
    auto node_rule { node->node_rule };

    auto initial_diff { node_rule ? initial_credit_diff - initial_debit_diff : initial_debit_diff - initial_credit_diff };

    node->initial_total += initial_diff;
    UpdateLeafTotal(node);

    UpdateBranchTotal(node, initial_diff);
    emit SUpdateDSpinBox();
}

void TreeModelProduct::IniTree(NodeHash& node_hash, StringHash& leaf_path, StringHash& branch_path)
{
    sql_->Tree(node_hash);

    for (auto& node : std::as_const(node_hash)) {
        if (!node->parent) {
            node->parent = root_;
            root_->children.emplace_back(node);
        }
    }

    QString path {};
    for (auto& node : std::as_const(node_hash)) {
        path = CreatePath(node);

        if (node->branch) {
            branch_path.insert(node->id, path);
            continue;
        }

        UpdateBranchTotal(node, node->initial_total);
        leaf_path.insert(node->id, path);
    }

    node_hash.insert(-1, root_);
}

void TreeModelProduct::UpdateBranchTotal(const Node* node, double initial_diff)
{
    if (!node)
        return;

    bool equal {};
    int unit { node->unit };
    bool node_rule { node->node_rule };

    while (node != root_) {
        equal = node->parent->node_rule == node_rule;

        if (node->parent->unit == unit)
            node->parent->initial_total += equal ? initial_diff : -initial_diff;

        node = node->parent;
    }
}

QString TreeModelProduct::CreatePath(const Node* node) const
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

void TreeModelProduct::UpdateBranchUnit(Node* node) const
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

void TreeModelProduct::RecycleNode(NodeHash& node_hash)
{
    for (auto& node : std::as_const(node_hash))
        NodePool::Instance().Recycle(node);

    node_hash.clear();
}

QModelIndex TreeModelProduct::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    auto parent_node { GetNode(parent) };
    auto node { parent_node->children.at(row) };

    return node ? createIndex(row, column, node) : QModelIndex();
}

QModelIndex TreeModelProduct::parent(const QModelIndex& index) const
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

int TreeModelProduct::rowCount(const QModelIndex& parent) const { return GetNode(parent)->children.size(); }

bool TreeModelProduct::UpdateBranch(Node* node, bool value)
{
    int node_id { node->id };
    if (node->branch == value || !node->children.isEmpty() || table_hash_->contains(node_id))
        return false;

    if (sql_->InternalReferences(node_id) || sql_->ExternalReferences(node_id, Section::kStakeholder) || sql_->ExternalReferences(node_id, Section::kPurchase)
        || sql_->ExternalReferences(node_id, Section::kSales))
        return false;

    node->branch = value;
    sql_->Update(info_->node, BRANCH, value, node_id);

    (node->branch) ? branch_path_.insert(node_id, leaf_path_.take(node_id)) : leaf_path_.insert(node_id, branch_path_.take(node_id));
    return true;
}

bool TreeModelProduct::UpdateDescription(Node* node, CString& value)
{
    if (node->description == value)
        return false;

    node->description = value;
    sql_->Update(info_->node, DESCRIPTION, value, node->id);
    return true;
}

bool TreeModelProduct::UpdateNote(Node* node, CString& value)
{
    if (node->note == value)
        return false;

    node->note = value;
    sql_->Update(info_->node, NOTE, value, node->id);
    return true;
}

bool TreeModelProduct::UpdateUnit(Node* node, int value)
{
    int node_id { node->id };
    if (node->unit == value || sql_->InternalReferences(node_id))
        return false;

    node->unit = value;
    sql_->Update(info_->node, UNIT, value, node_id);

    if (node->branch)
        UpdateBranchUnit(node);

    return true;
}

bool TreeModelProduct::UpdateName(Node* node, CString& value)
{
    node->name = value;
    sql_->Update(info_->node, NAME, value, node->id);

    UpdatePath(node);
    emit SResizeColumnToContents(std::to_underlying(TreeColumn::kName));
    emit SSearch();
    return true;
}

bool TreeModelProduct::UpdateUnitPrice(Node* node, double value)
{
    const double tolerance { std::pow(10, -section_rule_->ratio_decimal - 2) };

    if (std::abs(node->third_property - value) < tolerance)
        return false;

    node->third_property = value;
    sql_->Update(info_->node, UNIT_PRICE, value, node->id);

    return true;
}

bool TreeModelProduct::UpdateCommission(Node* node, double value)
{
    const double tolerance { std::pow(10, -section_rule_->ratio_decimal - 2) };

    if (std::abs(node->fourth_property - value) < tolerance)
        return false;

    node->fourth_property = value;
    sql_->Update(info_->node, COMMISSION, value, node->id);

    return true;
}

void TreeModelProduct::UpdatePath(const Node* node)
{
    QQueue<const Node*> queue {};
    queue.enqueue(node);

    const Node* queue_node {};
    QString path {};

    while (!queue.isEmpty()) {
        queue_node = queue.dequeue();

        path = CreatePath(queue_node);

        if (queue_node->branch) {
            for (const auto& child : queue_node->children)
                queue.enqueue(child);

            branch_path_.insert(queue_node->id, path);
            continue;
        }

        leaf_path_.insert(queue_node->id, path);
    }
}

bool TreeModelProduct::UpdateRule(Node* node, bool value)
{
    if (node->node_rule == value)
        return false;

    node->node_rule = value;
    sql_->Update(info_->node, NODE_RULE, value, node->id);

    node->initial_total = -node->initial_total;
    if (!node->branch)
        emit SNodeRule(node->id, value);

    return true;
}

bool TreeModelProduct::UpdateCode(Node* node, CString& value)
{
    if (node->code == value)
        return false;

    node->code = value;
    sql_->Update(info_->node, CODE, value, node->id);
    return true;
}

void TreeModelProduct::sort(int column, Qt::SortOrder order)
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
        case TreeColumn::kThird:
            return (order == Qt::AscendingOrder) ? (lhs->third_property < rhs->third_property) : (lhs->third_property > rhs->third_property);
        case TreeColumn::kInitialTotal:
            return (order == Qt::AscendingOrder) ? (lhs->initial_total < rhs->initial_total) : (lhs->initial_total > rhs->initial_total);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    SortIterative(root_, Compare);
    emit layoutChanged();
}

void TreeModelProduct::SortIterative(Node* node, std::function<bool(const Node*, const Node*)> Compare)
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

Node* TreeModelProduct::GetNode(const QModelIndex& index) const
{
    if (index.isValid() && index.internalPointer())
        return static_cast<Node*>(index.internalPointer());

    return root_;
}

QString TreeModelProduct::Path(int node_id) const
{
    auto path { leaf_path_.value(node_id) };

    if (path.isNull())
        path = branch_path_.value(node_id);

    return path;
}

QModelIndex TreeModelProduct::GetIndex(int node_id) const
{
    if (node_id == -1 || !node_hash_.contains(node_id))
        return QModelIndex();

    auto node { node_hash_.value(node_id) };
    return createIndex(node->parent->children.indexOf(node), 0, node);
}

bool TreeModelProduct::IsDescendant(Node* lhs, Node* rhs) const
{
    if (!lhs || !rhs || lhs == rhs)
        return false;

    while (lhs && lhs != rhs)
        lhs = lhs->parent;

    return lhs == rhs;
}

bool TreeModelProduct::InsertRow(int row, const QModelIndex& parent, Node* node)
{
    if (row <= -1)
        return false;

    auto parent_node { GetNode(parent) };

    beginInsertRows(parent, row, row);
    parent_node->children.insert(row, node);
    endInsertRows();

    sql_->Insert(parent_node->id, node);
    node_hash_.insert(node->id, node);

    QString path { CreatePath(node) };
    (node->branch ? branch_path_ : leaf_path_).insert(node->id, path);

    emit SSearch();
    return true;
}

bool TreeModelProduct::RemoveRow(int row, const QModelIndex& parent)
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
        UpdatePath(node);
        branch_path_.remove(node_id);
        sql_->Remove(node_id, true);
        emit SUpdateName(node);
    }

    if (!branch) {
        UpdateBranchTotal(node, -node->initial_total);
        leaf_path_.remove(node_id);
        sql_->Remove(node_id, false);
    }

    emit SSearch();
    emit SResizeColumnToContents(std::to_underlying(TreeColumn::kName));

    NodePool::Instance().Recycle(node);
    node_hash_.remove(node_id);

    return true;
}

bool TreeModelProduct::UpdateLeafTotal(const Node* node)
{
    if (!node || node->branch)
        return false;

    sql_->Update(info_->node, INITIAL_TOTAL, node->initial_total, node->id);

    return true;
}

QVariant TreeModelProduct::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return info_->tree_header.at(section);

    return QVariant();
}

QVariant TreeModelProduct::data(const QModelIndex& index, int role) const
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
    case TreeColumn::kCode:
        return node->code;
    case TreeColumn::kThird:
        return node->third_property == 0 ? QVariant() : node->third_property;
    case TreeColumn::kFourth:
        return node->fourth_property == 0 ? QVariant() : node->fourth_property;
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
        return node->initial_total;
    default:
        return QVariant();
    }
}

bool TreeModelProduct::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    auto node { GetNode(index) };
    if (node->id == -1)
        return false;

    const TreeColumn kColumn { index.column() };

    switch (kColumn) {
    case TreeColumn::kDescription:
        UpdateDescription(node, value.toString());
        break;
    case TreeColumn::kNote:
        UpdateNote(node, value.toString());
        break;
    case TreeColumn::kCode:
        UpdateCode(node, value.toString());
        break;
    case TreeColumn::kThird:
        UpdateUnitPrice(node, value.toDouble());
        break;
    case TreeColumn::kFourth:
        UpdateCommission(node, value.toDouble());
        break;
    case TreeColumn::kNodeRule:
        UpdateRule(node, value.toBool());
        break;
    case TreeColumn::kUnit:
        UpdateUnit(node, value.toInt());
        break;
    case TreeColumn::kBranch:
        UpdateBranch(node, value.toBool());
        break;
    default:
        return false;
    }

    emit SResizeColumnToContents(index.column());
    return true;
}

Qt::ItemFlags TreeModelProduct::flags(const QModelIndex& index) const
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
    case TreeColumn::kInitialTotal:
        flags &= ~Qt::ItemIsEditable;
        break;
    default:
        flags |= Qt::ItemIsEditable;
        break;
    }

    return flags;
}

QMimeData* TreeModelProduct::mimeData(const QModelIndexList& indexes) const
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

bool TreeModelProduct::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex&) const
{
    Q_UNUSED(row);
    Q_UNUSED(column);

    return action != Qt::IgnoreAction && data && data->hasFormat(NODE_ID);
}

bool TreeModelProduct::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
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

    auto begin_row { row == -1 ? destination_parent->children.size() : row };
    auto source_row { node->parent->children.indexOf(node) };
    auto source_index { createIndex(node->parent->children.indexOf(node), 0, node) };

    if (beginMoveRows(source_index.parent(), source_row, source_row, parent, begin_row)) {
        node->parent->children.removeAt(source_row);
        UpdateBranchTotal(node, -node->initial_total);

        destination_parent->children.insert(begin_row, node);
        node->parent = destination_parent;
        UpdateBranchTotal(node, node->initial_total);

        endMoveRows();
    }

    sql_->Drag(destination_parent->id, node_id);
    UpdatePath(node);
    emit SResizeColumnToContents(std::to_underlying(TreeColumn::kName));
    emit SUpdateName(node);

    return true;
}
