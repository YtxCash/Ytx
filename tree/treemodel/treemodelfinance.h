#ifndef TREEMODELFINANCE_H
#define TREEMODELFINANCE_H

#include "component/info.h"
#include "component/settings.h"
#include "sql/sql.h"
#include "tree/abstracttreemodel.h"

class TreeModelFinance final : public AbstractTreeModel {
    Q_OBJECT

public:
    TreeModelFinance(const Info* info, QSharedPointer<Sql> sql, const SectionRule* section_rule, const TableHash* table_hash, const Interface* interface,
        QObject* parent = nullptr);
    ~TreeModelFinance() override;

public slots:
    // receive from table sql
    bool RUpdateMultiTotal(const QList<int>& node_list) override;
    // receive from related table model
    void RUpdateOneTotal(int node_id, double final_debit_diff, double final_credit_diff, double initial_debit_diff, double initial_credit_diff) override;
    void RUpdateProperty(int, double, double, double) override { }
    // receive from table model
    void RSearch() override { emit SSearch(); }
    // receive from table sql
    bool RRemoveNode(int node_id) override;

public:
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override
    {
        Q_UNUSED(parent);
        return info_->tree_header.size();
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    void sort(int column, Qt::SortOrder order) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    Qt::DropActions supportedDropActions() const override { return Qt::CopyAction | Qt::MoveAction; }
    QStringList mimeTypes() const override { return QStringList() << "node/id"; }

    QMimeData* mimeData(const QModelIndexList& indexes) const override;
    bool canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const override;

    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;

public:
    void UpdateNode(const Node* tmp_node) override;
    bool RemoveRow(int row, const QModelIndex& parent = QModelIndex()) override;

    bool InsertRow(int row, const QModelIndex& parent, Node* node) override;

    CStringHash* LeafPath() const override { return &leaf_path_; }
    CStringHash* BranchPath() const override { return &branch_path_; }
    const NodeHash* GetNodeHash() const override { return &node_hash_; }
    const Node* GetNode(int node_id) const override { return node_hash_.value(node_id, nullptr); }

    Node* GetNode(const QModelIndex& index) const override;
    QModelIndex GetIndex(int node_id) const override;
    QString Path(int node_id) const override;

    void UpdateBranchUnit(Node* node) const override;
    void UpdateSeparator(CString& separator) override;

private:
    void IniTree(NodeHash& node_hash, StringHash& leaf_path, StringHash& branch_path);
    void UpdateBranchTotal(const Node* node, double final_diff, double initial_diff);
    bool UpdateLeafTotal(const Node* node); // jsus store leaf's total into sqlite3 table

    bool UpdateBranch(Node* node, bool value);
    bool UpdateRule(Node* node, bool value);
    bool UpdateCode(Node* node, CString& value);
    bool UpdateDescription(Node* node, CString& value);
    bool UpdateNote(Node* node, CString& value);
    bool UpdateUnit(Node* node, int value);
    bool UpdateName(Node* node, CString& value);

    void UpdatePath(const Node* node);
    bool IsDescendant(Node* lhs, Node* rhs) const;

    QString CreatePath(const Node* node) const;
    void SortIterative(Node* node, std::function<bool(const Node*, const Node*)> Compare);
    void RecycleNode(NodeHash& node_hash);

private:
    const Info* info_ {};
    const SectionRule* section_rule_ {};
    const TableHash* table_hash_ {};
    const Interface* interface_ {};
    QSharedPointer<Sql> sql_ {};

    Node* root_ {};

    NodeHash node_hash_ {};
    StringHash leaf_path_ {};
    StringHash branch_path_ {};
};

#endif // TREEMODELFINANCE_H
