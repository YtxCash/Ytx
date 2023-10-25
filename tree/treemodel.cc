#include "treemodel.h"

#include <QMimeData>
#include <QQueue>
#include <QRegularExpression>

#include "component/constvalue.h"
#include "component/enumclass.h"
#include "global/nodepool.h"

TreeModel::TreeModel(
    const TreeInfo* info, TreeSql* sql, const SectionRule* section_rule, const ViewHash* sectioon_view, const Interface* interface, QObject* parent)
    : QAbstractItemModel { parent }
    , info_ { info }
    , section_rule_ { section_rule }
    , section_view_ { sectioon_view }
    , interface_ { interface }
    , sql_ { sql }
{
    root_ = NodePool::Instance().Allocate();
    root_->id = -1;
    root_->branch = true;
    root_->unit = section_rule->base_unit;

    IniTree(node_hash_, leaf_path_, branch_path_);
}

TreeModel::~TreeModel() { RecycleNode(node_hash_); }

void TreeModel::UpdateNode(const Node* tmp_node)
{
    if (!tmp_node)
        return;

    auto node { node_hash_.value(tmp_node->id) };
    if (*node == *tmp_node)
        return;

    UpdateBranch(node, tmp_node->branch);
    UpdateCode(node, tmp_node->code);
    UpdateDescription(node, tmp_node->description);
    UpdateNote(node, tmp_node->note);
    UpdateRule(node, tmp_node->node_rule);
    UpdateUnit(node, tmp_node->unit);

    if (node->name != tmp_node->name)
        UpdateName(node, tmp_node->name);
}

void TreeModel::UpdateSeparator(CString& separator)
{
    if (interface_->separator == separator || separator.isEmpty())
        return;

    for (auto& path : leaf_path_)
        path.replace(interface_->separator, separator);

    for (auto& path : branch_path_)
        path.replace(interface_->separator, separator);
}

bool TreeModel::RRemoveNode(int node_id)
{
    auto index { GetIndex(node_id) };
    auto row { index.row() };
    auto parent_index { index.parent() };
    RemoveRow(row, parent_index);

    return true;
}

void TreeModel::IniTree(NodeHash& node_hash, StringHash& leaf_path, StringHash& branch_path)
{
    sql_->Tree(node_hash);

    for (auto node : std::as_const(node_hash)) {
        if (!node->parent) {
            node->parent = root_;
            root_->children.emplace_back(node);
        }
    }

    QString path {};
    for (auto node : std::as_const(node_hash)) {
        path = CreatePath(node);

        if (node->branch) {
            branch_path.insert(node->id, path);
            continue;
        }

        sql_->LeafTotal(node);
        UpdateBranchTotal(node, node->base_total, node->foreign_total);
        leaf_path.insert(node->id, path);
    }

    node_hash.insert(-1, root_);
}

QString TreeModel::CreatePath(const Node* node) const
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

void TreeModel::UpdateBranchTotal(const Node* node, double base_diff, double foreign_diff)
{
    if (!node)
        return;

    bool equal {};
    int unit { node->unit };
    bool node_rule { node->node_rule };

    while (node != root_) {
        equal = node->parent->node_rule == node_rule;
        node->parent->base_total += equal ? base_diff : -base_diff;

        if (node->parent->unit == unit)
            node->parent->foreign_total += equal ? foreign_diff : -foreign_diff;

        node = node->parent;
    }
}

void TreeModel::UpdateLeafUnit(const Node* node, int unit, int value)
{
    if (!node || node->branch)
        return;

    auto minus(node);
    auto plus(node);

    bool equal {};
    bool node_rule { node->node_rule };
    auto foreign_total { node->foreign_total };
    auto base_unit { section_rule_->base_unit };

    if (unit != base_unit) {
        while (minus != root_) {
            if (minus->parent->unit == unit) {
                equal = minus->parent->node_rule == node_rule;
                minus->parent->foreign_total -= equal ? foreign_total : -foreign_total;
            }

            minus = minus->parent;
        }
    }

    if (value != base_unit) {
        while (plus != root_) {
            if (plus->parent->unit == value) {
                equal = plus->parent->node_rule == node_rule;
                plus->parent->foreign_total += equal ? foreign_total : -foreign_total;
            }

            plus = plus->parent;
        }
    }
}

bool TreeModel::RUpdateMultiTotal(const QList<int>& node_list)
{
    double old_base_total {};
    double old_foreign_total {};
    double base_diff {};
    double foreign_diff {};
    Node* node {};

    for (const int& node_id : node_list) {
        node = const_cast<Node*>(GetNode(node_id));

        if (!node || node->branch)
            continue;

        old_base_total = node->base_total;
        old_foreign_total = node->foreign_total;

        sql_->LeafTotal(node);

        base_diff = node->base_total - old_base_total;
        foreign_diff = node->foreign_total - old_foreign_total;

        UpdateBranchTotal(node, base_diff, foreign_diff);
    }

    emit SUpdateStatusBarSpin();
    return true;
}

void TreeModel::RUpdateOneTotal(int node_id, double base_debit_diff, double base_credit_diff, double foreign_debit_diff, double foreign_credit_diff)
{
    auto node { const_cast<Node*>(GetNode(node_id)) };
    auto node_rule { node->node_rule };

    auto base_diff { node_rule ? base_credit_diff - base_debit_diff : base_debit_diff - base_credit_diff };
    auto foreign_diff { node_rule ? foreign_credit_diff - foreign_debit_diff : foreign_debit_diff - foreign_credit_diff };

    node->base_total += base_diff;
    node->foreign_total += foreign_diff;

    UpdateBranchTotal(node, base_diff, foreign_diff);
    emit SUpdateStatusBarSpin();
}

void TreeModel::UpdateBranchUnit(Node* node) const
{
    if (!node || !node->branch || node->unit == section_rule_->base_unit)
        return;

    QQueue<const Node*> queue {};
    queue.enqueue(node);

    const Node* queue_node {};
    double foreign_total {};
    bool equal {};
    bool branch {};

    int unit { node->unit };
    bool node_rule { node->node_rule };

    while (!queue.isEmpty()) {
        queue_node = queue.dequeue();
        branch = queue_node->branch;

        if (branch)
            for (const auto* child : queue_node->children)
                queue.enqueue(child);

        if (!branch && queue_node->unit == unit) {
            equal = queue_node->node_rule == node_rule;
            foreign_total += equal ? queue_node->foreign_total : -queue_node->foreign_total;
        }
    }

    node->foreign_total = foreign_total;
}

void TreeModel::RecycleNode(NodeHash& node_hash)
{
    for (auto node : std::as_const(node_hash))
        NodePool::Instance().Recycle(node);

    node_hash.clear();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    auto parent_node { GetNode(parent) };
    auto node { parent_node->children.at(row) };

    return node ? createIndex(row, column, node) : QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex& index) const
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

int TreeModel::rowCount(const QModelIndex& parent) const { return GetNode(parent)->children.size(); }

QVariant TreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto node { GetNode(index) };
    if (node == root_)
        return QVariant();

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

bool TreeModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    auto node { GetNode(index) };
    if (node == root_)
        return false;

    const NodeColumn kColumn { index.column() };

    switch (kColumn) {
    case NodeColumn::kDescription:
        UpdateDescription(node, value.toString());
        break;
    case NodeColumn::kNote:
        UpdateNote(node, value.toString());
        break;
    case NodeColumn::kCode:
        UpdateCode(node, value.toString());
        break;
    case NodeColumn::kNodeRule:
        UpdateRule(node, value.toBool());
        break;
    case NodeColumn::kUnit:
        UpdateUnit(node, value.toInt());
        break;
    case NodeColumn::kBranch:
        UpdateBranch(node, value.toBool());
        break;
    default:
        return false;
    }

    emit SResizeColumnToContents(index.column());
    return true;
}

bool TreeModel::UpdateBranch(Node* node, bool value)
{
    int node_id { node->id };
    if (node->branch == value || !node->children.isEmpty() || sql_->Usage(node_id) || section_view_->contains(node_id))
        return false;

    node->branch = value;
    sql_->Update(BRANCH, value, node_id);

    (node->branch) ? branch_path_.insert(node_id, leaf_path_.take(node_id)) : leaf_path_.insert(node_id, branch_path_.take(node_id));
    return true;
}

bool TreeModel::UpdateDescription(Node* node, CString& value)
{
    if (node->description == value)
        return false;

    node->description = value;
    sql_->Update(DESCRIPTION, value, node->id);
    return true;
}

bool TreeModel::UpdateNote(Node* node, CString& value)
{
    if (node->note == value)
        return false;

    node->note = value;
    sql_->Update(NOTE, value, node->id);
    return true;
}

bool TreeModel::UpdateUnit(Node* node, int value)
{
    auto unit { node->unit };
    if (unit == value)
        return false;

    node->unit = value;
    sql_->Update(UNIT, value, node->id);

    (node->branch) ? UpdateBranchUnit(node) : UpdateLeafUnit(node, unit, value);
    return true;
}

bool TreeModel::UpdateName(Node* node, CString& value)
{
    node->name = value;
    sql_->Update(NAME, value, node->id);

    UpdatePath(node);
    emit SResizeColumnToContents(static_cast<int>(NodeColumn::kName));
    emit SSearch();
    emit SUpdateName(node);
    return true;
}

void TreeModel::UpdatePath(const Node* node)
{
    QQueue<const Node*> queue {};
    queue.enqueue(node);

    const Node* queue_node {};
    QString path {};

    while (!queue.isEmpty()) {
        queue_node = queue.dequeue();

        path = CreatePath(queue_node);

        if (queue_node->branch) {
            for (const auto* child : queue_node->children)
                queue.enqueue(child);

            branch_path_.insert(queue_node->id, path);
            continue;
        }

        leaf_path_.insert(queue_node->id, path);
    }
}

bool TreeModel::UpdateRule(Node* node, bool value)
{
    if (node->node_rule == value)
        return false;

    node->node_rule = value;
    sql_->Update(NODERULE, value, node->id);

    node->base_total = -node->base_total;
    node->foreign_total = -node->foreign_total;
    if (!node->branch)
        emit SNodeRule(node->id, value);

    return true;
}

bool TreeModel::UpdateCode(Node* node, CString& value)
{
    if (node->code == value)
        return false;

    node->code = value;
    sql_->Update(CODE, value, node->id);
    return true;
}

int TreeModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return info_->header.size();
}

void TreeModel::sort(int column, Qt::SortOrder order)
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
    SortIterative(root_, Compare);
    emit layoutChanged();
}

void TreeModel::SortIterative(Node* node, const auto& Compare)
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
            for (auto child : queue_node->children) {
                queue.enqueue(child);
            }
        }
    }
}

Node* TreeModel::GetNode(const QModelIndex& index) const
{
    if (index.isValid() && index.internalPointer())
        return static_cast<Node*>(index.internalPointer());

    return root_;
}

QString TreeModel::Path(int node_id) const
{
    auto path { leaf_path_.value(node_id) };

    if (path.isNull())
        path = branch_path_.value(node_id);

    return path;
}

QModelIndex TreeModel::GetIndex(int node_id) const
{
    if (node_id == -1 || !node_hash_.contains(node_id))
        return QModelIndex();

    auto node { node_hash_.value(node_id) };
    return createIndex(node->parent->children.indexOf(node), 0, node);
}

bool TreeModel::IsDescendant(Node* lhs, Node* rhs) const
{
    if (!lhs || !rhs || lhs == rhs)
        return false;

    while (lhs && lhs != rhs)
        lhs = lhs->parent;

    return lhs == rhs;
}

bool TreeModel::InsertRow(int row, const QModelIndex& parent, Node* node)
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

bool TreeModel::RemoveRow(int row, const QModelIndex& parent)
{
    if (row <= -1 || row >= rowCount(parent))
        return false;

    auto parent_node { GetNode(parent) };
    auto node { parent_node->children.at(row) };

    int node_id { node->id };
    bool branch { node->branch };

    beginRemoveRows(parent, row, row);
    if (branch) {
        for (auto child : node->children) {
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
        UpdateBranchTotal(node, -node->base_total, -node->foreign_total);
        leaf_path_.remove(node_id);
        sql_->Remove(node_id, false);
    }

    emit SSearch();
    emit SResizeColumnToContents(static_cast<int>(NodeColumn::kName));

    NodePool::Instance().Recycle(node);
    node_hash_.remove(node_id);

    return true;
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return info_->header.at(section);

    return QVariant();
}

Qt::ItemFlags TreeModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    auto flags { QAbstractItemModel::flags(index) };
    const NodeColumn kColumn { index.column() };

    switch (kColumn) {
    case NodeColumn::kName:
        flags |= Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
        break;
    case NodeColumn::kCode:
        flags |= Qt::ItemIsEditable;
        break;
    case NodeColumn::kDescription:
        flags |= Qt::ItemIsEditable;
        break;
    case NodeColumn::kNote:
        flags |= Qt::ItemIsEditable;
        break;
    case NodeColumn::kNodeRule:
        flags |= Qt::ItemIsEditable;
        break;
    case NodeColumn::kBranch:
        flags |= Qt::ItemIsEditable;
        break;
    case NodeColumn::kUnit:
        flags |= Qt::ItemIsEditable;
    default:
        break;
    }

    return flags;
}

QMimeData* TreeModel::mimeData(const QModelIndexList& indexes) const
{
    auto mime_data { new QMimeData() };
    if (indexes.isEmpty())
        return mime_data;

    auto first_index = indexes.first();

    if (first_index.isValid()) {
        int id { first_index.sibling(first_index.row(), static_cast<int>(NodeColumn::kID)).data().toInt() };
        mime_data->setData(NODEID, QByteArray::number(id));
    }

    return mime_data;
}

bool TreeModel::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex&) const
{
    Q_UNUSED(row);
    Q_UNUSED(column);

    return action != Qt::IgnoreAction && data && data->hasFormat(NODEID);
}

bool TreeModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
    if (!canDropMimeData(data, action, row, column, parent))
        return false;

    auto destination_parent { GetNode(parent) };
    if (!destination_parent->branch)
        return false;

    int node_id {};

    if (auto mime { data->data(NODEID) }; !mime.isEmpty())
        node_id = QVariant(mime).toInt();

    auto node { const_cast<Node*>(GetNode(node_id)) };
    if (!node || node->parent == destination_parent || IsDescendant(destination_parent, node))
        return false;

    auto begin_row { row == -1 ? destination_parent->children.size() : row };
    auto source_row { node->parent->children.indexOf(node) };
    auto source_index { createIndex(node->parent->children.indexOf(node), 0, node) };

    if (beginMoveRows(source_index.parent(), source_row, source_row, parent, begin_row)) {
        node->parent->children.removeAt(source_row);
        UpdateBranchTotal(node, -node->base_total, -node->foreign_total);

        destination_parent->children.insert(begin_row, node);
        node->parent = destination_parent;
        UpdateBranchTotal(node, node->base_total, node->foreign_total);

        endMoveRows();
    }

    sql_->Drag(destination_parent->id, node_id);
    UpdatePath(node);
    emit SResizeColumnToContents(static_cast<int>(NodeColumn::kName));
    emit SSearch();
    emit SUpdateName(node);

    return true;
}
