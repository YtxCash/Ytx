#include "treemodelstakeholder.h"

#include <QMimeData>
#include <QQueue>
#include <QRegularExpression>

#include "component/constvalue.h"
#include "component/enumclass.h"
#include "global/nodepool.h"

TreeModelStakeholder::TreeModelStakeholder(
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

TreeModelStakeholder::~TreeModelStakeholder() { RecycleNode(node_hash_); }

bool TreeModelStakeholder::RRemoveNode(int node_id)
{
    auto index { GetIndex(node_id) };
    auto row { index.row() };
    auto parent_index { index.parent() };
    RemoveRow(row, parent_index);

    return true;
}

void TreeModelStakeholder::UpdateNode(const Node* tmp_node)
{
    if (!tmp_node)
        return;

    auto node { const_cast<Node*>(GetNode(tmp_node->id)) };
    UpdateTaxRate(node, tmp_node->third_property);

    if (*node == *tmp_node)
        return;

    UpdateBranch(node, tmp_node->branch);
    UpdateCode(node, tmp_node->code);
    UpdateDescription(node, tmp_node->description);
    UpdateNote(node, tmp_node->note);
    UpdateDeadline(node, tmp_node->date_time);
    UpdatePaymentPeriod(node, tmp_node->first_property);
    UpdateTerm(node, tmp_node->node_rule);
    UpdateDecimal(node, tmp_node->second_property);

    if (node->name != tmp_node->name) {
        UpdateName(node, tmp_node->name);
        emit SUpdateName(node);
    }
}

bool TreeModelStakeholder::UpdateTaxRate(Node* node, double value)
{
    const double tolerance { std::pow(10, -section_rule_->ratio_decimal - 2) };

    if (std::abs(node->third_property - value) < tolerance)
        return false;

    node->third_property = value;
    sql_->Update(info_->node, TAX_RATE, value, node->id);

    return true;
}

bool TreeModelStakeholder::UpdatePaymentPeriod(Node* node, int value)
{
    if (node->first_property == value)
        return false;

    node->first_property = value;
    sql_->Update(info_->node, PAYMENT_PERIOD, value, node->id);
    return true;
}

bool TreeModelStakeholder::UpdateDeadline(Node* node, CString& value)
{
    if (node->date_time == value)
        return false;

    node->date_time = value;
    sql_->Update(info_->node, DEADLINE, value, node->id);
    return true;
}

void TreeModelStakeholder::UpdateSeparator(CString& separator)
{
    if (interface_->separator == separator || separator.isEmpty())
        return;

    for (auto& path : leaf_path_)
        path.replace(interface_->separator, separator);

    for (auto& path : branch_path_)
        path.replace(interface_->separator, separator);
}

void TreeModelStakeholder::IniTree(NodeHash& node_hash, StringHash& leaf_path, StringHash& branch_path)
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

        leaf_path.insert(node->id, path);
    }

    node_hash.insert(-1, root_);
}

QString TreeModelStakeholder::CreatePath(const Node* node) const
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

void TreeModelStakeholder::RecycleNode(NodeHash& node_hash)
{
    for (auto& node : std::as_const(node_hash))
        NodePool::Instance().Recycle(node);

    node_hash.clear();
}

QModelIndex TreeModelStakeholder::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    auto parent_node { GetNode(parent) };
    auto node { parent_node->children.at(row) };

    return node ? createIndex(row, column, node) : QModelIndex();
}

QModelIndex TreeModelStakeholder::parent(const QModelIndex& index) const
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

int TreeModelStakeholder::rowCount(const QModelIndex& parent) const { return GetNode(parent)->children.size(); }

bool TreeModelStakeholder::UpdateBranch(Node* node, bool value)
{
    int node_id { node->id };
    if (node->branch == value || !node->children.isEmpty() || table_hash_->contains(node_id))
        return false;

    if (sql_->InternalReferences(node_id) || sql_->ExternalReferences(node_id, Section::kPurchase) || sql_->ExternalReferences(node_id, Section::kSales))
        return false;

    node->branch = value;
    sql_->Update(info_->node, BRANCH, value, node_id);

    (node->branch) ? branch_path_.insert(node_id, leaf_path_.take(node_id)) : leaf_path_.insert(node_id, branch_path_.take(node_id));
    return true;
}

bool TreeModelStakeholder::UpdateTerm(Node* node, bool value)
{
    if (node->node_rule == value)
        return false;

    node->node_rule = value;
    sql_->Update(info_->node, TERM, value, node->id);
    return true;
}

bool TreeModelStakeholder::UpdateDecimal(Node* node, int value)
{
    if (node->second_property == value)
        return false;

    node->second_property = value;
    sql_->Update(info_->node, DECIMAL, value, node->id);
    return true;
}

bool TreeModelStakeholder::UpdateDescription(Node* node, CString& value)
{
    if (node->description == value)
        return false;

    node->description = value;
    sql_->Update(info_->node, DESCRIPTION, value, node->id);
    return true;
}

bool TreeModelStakeholder::UpdateNote(Node* node, CString& value)
{
    if (node->note == value)
        return false;

    node->note = value;
    sql_->Update(info_->node, NOTE, value, node->id);
    return true;
}

bool TreeModelStakeholder::UpdateName(Node* node, CString& value)
{
    node->name = value;
    sql_->Update(info_->node, NAME, value, node->id);

    UpdatePath(node);
    emit SResizeColumnToContents(std::to_underlying(TreeColumn::kName));
    emit SSearch();
    return true;
}

void TreeModelStakeholder::UpdatePath(const Node* node)
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

bool TreeModelStakeholder::UpdateCode(Node* node, CString& value)
{
    if (node->code == value)
        return false;

    node->code = value;
    sql_->Update(info_->node, CODE, value, node->id);
    return true;
}

void TreeModelStakeholder::sort(int column, Qt::SortOrder order)
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
        case TreeColumn::kFirst:
            return (order == Qt::AscendingOrder) ? (lhs->first_property < rhs->first_property) : (lhs->first_property > rhs->first_property);
        case TreeColumn::kDateTime:
            return (order == Qt::AscendingOrder) ? (lhs->date_time < rhs->date_time) : (lhs->date_time > rhs->date_time);
        default:
            return false;
        }
    };

    emit layoutAboutToBeChanged();
    SortIterative(root_, Compare);
    emit layoutChanged();
}

void TreeModelStakeholder::SortIterative(Node* node, std::function<bool(const Node*, const Node*)> Compare)
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

Node* TreeModelStakeholder::GetNode(const QModelIndex& index) const
{
    if (index.isValid() && index.internalPointer())
        return static_cast<Node*>(index.internalPointer());

    return root_;
}

QString TreeModelStakeholder::Path(int node_id) const
{
    auto path { leaf_path_.value(node_id) };

    if (path.isNull())
        path = branch_path_.value(node_id);

    return path;
}

QModelIndex TreeModelStakeholder::GetIndex(int node_id) const
{
    if (node_id == -1 || !node_hash_.contains(node_id))
        return QModelIndex();

    auto node { node_hash_.value(node_id) };
    return createIndex(node->parent->children.indexOf(node), 0, node);
}

bool TreeModelStakeholder::IsDescendant(Node* lhs, Node* rhs) const
{
    if (!lhs || !rhs || lhs == rhs)
        return false;

    while (lhs && lhs != rhs)
        lhs = lhs->parent;

    return lhs == rhs;
}

bool TreeModelStakeholder::InsertRow(int row, const QModelIndex& parent, Node* node)
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

bool TreeModelStakeholder::RemoveRow(int row, const QModelIndex& parent)
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
    }

    if (!branch) {
        leaf_path_.remove(node_id);
        sql_->Remove(node_id, false);
    }

    emit SSearch();
    emit SResizeColumnToContents(std::to_underlying(TreeColumn::kName));

    NodePool::Instance().Recycle(node);
    node_hash_.remove(node_id);

    return true;
}

QVariant TreeModelStakeholder::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return info_->tree_header.at(section);

    return QVariant();
}

QVariant TreeModelStakeholder::data(const QModelIndex& index, int role) const
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
    case TreeColumn::kDescription:
        return node->description;
    case TreeColumn::kThird:
        return node->third_property == 0 ? QVariant() : node->third_property;
    case TreeColumn::kFirst:
        return node->first_property == 0 ? QVariant() : node->first_property;
    case TreeColumn::kSecond:
        return node->second_property == 0 ? QVariant() : node->second_property;
    case TreeColumn::kDateTime:
        return node->date_time;
    case TreeColumn::kNote:
        return node->note;
    case TreeColumn::kNodeRule:
        return node->node_rule;
    case TreeColumn::kUnit:
        return node->unit;
    case TreeColumn::kBranch:
        return node->branch;
    default:
        return QVariant();
    }
}

bool TreeModelStakeholder::setData(const QModelIndex& index, const QVariant& value, int role)
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
    case TreeColumn::kNodeRule:
        UpdateTerm(node, value.toBool());
        break;
    case TreeColumn::kCode:
        UpdateCode(node, value.toString());
        break;
    case TreeColumn::kThird:
        UpdateTaxRate(node, value.toDouble());
        break;
    case TreeColumn::kFirst:
        UpdatePaymentPeriod(node, value.toInt());
        break;
    case TreeColumn::kDateTime:
        UpdateDeadline(node, value.toString());
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

Qt::ItemFlags TreeModelStakeholder::flags(const QModelIndex& index) const
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
    case TreeColumn::kUnit:
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

QMimeData* TreeModelStakeholder::mimeData(const QModelIndexList& indexes) const
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

bool TreeModelStakeholder::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex&) const
{
    Q_UNUSED(row);
    Q_UNUSED(column);

    return action != Qt::IgnoreAction && data && data->hasFormat(NODE_ID);
}

bool TreeModelStakeholder::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
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

        destination_parent->children.insert(begin_row, node);
        node->parent = destination_parent;

        endMoveRows();
    }

    sql_->Drag(destination_parent->id, node_id);
    UpdatePath(node);
    emit SResizeColumnToContents(std::to_underlying(TreeColumn::kName));

    return true;
}
