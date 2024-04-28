#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QAbstractItemModel>

#include "component/settings.h"
#include "component/using.h"
#include "sql/treesql.h"
#include "treeinfo.h"

class TreeModel : public QAbstractItemModel {
    Q_OBJECT

public:
    TreeModel(const TreeInfo* info, TreeSql* sql, const SectionRule* section_rule, const ViewHash* sectioon_view, const Interface* interface,
        QObject* parent = nullptr);
    ~TreeModel();

signals:
    // send to related table model
    void SNodeRule(int node_id, bool node_rule);

    // send to its view
    void SResizeColumnToContents(int column);

    // send to search dialog
    void SSearch();

    // send to mainwindow
    void SUpdateName(const Node* node);
    void SUpdateStatusBarSpin();

public slots:
    // receive from table sql
    bool RUpdateMultiTotal(const QList<int>& node_list);
    bool RRemoveNode(int node_id);

    // receive from related table model
    void RUpdateOneTotal(int node_id, double base_debit_diff, double base_credit_diff, double foreign_debit_diff, double foreign_credit_diff);

    // receive from all table model
    void RSearch() { emit SSearch(); }

public:
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void sort(int column, Qt::SortOrder order) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    Qt::DropActions supportedDropActions() const override { return Qt::CopyAction | Qt::MoveAction; }
    QStringList mimeTypes() const override { return QStringList() << "node/id"; }

    QMimeData* mimeData(const QModelIndexList& indexes) const override;
    bool canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const override;
    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;

public:
    bool InsertRow(int row, const QModelIndex& parent, Node* node);
    bool RemoveRow(int row, const QModelIndex& parent = QModelIndex());

public:
    inline CStringHash* LeafPath() const { return &leaf_path_; }
    inline CStringHash* BranchPath() const { return &branch_path_; }
    inline const NodeHash* GetNodeHash() const { return &node_hash_; }
    inline const Node* GetNode(int node_id) const { return node_hash_.value(node_id, nullptr); }

public:
    Node* GetNode(const QModelIndex& index) const;
    QModelIndex GetIndex(int node_id) const;
    QString Path(int node_id) const;

    void UpdateBranchUnit(Node* node) const;
    void UpdateSeparator(CString& separator);
    void UpdateNode(const Node* tmp_node);

private:
    void IniTree(NodeHash& node_hash, StringHash& leaf_path, StringHash& branch_path);

    bool UpdateBranch(Node* node, bool value);
    bool UpdateRule(Node* node, bool value);
    bool UpdateCode(Node* node, CString& value);
    bool UpdateDescription(Node* node, CString& value);
    bool UpdateNote(Node* node, CString& value);
    bool UpdateUnit(Node* node, int value);
    bool UpdateName(Node* node, CString& value);

    void UpdatePath(const Node* node);
    void UpdateLeafUnit(const Node* node, int unit, int value);
    void UpdateBranchTotal(const Node* node, double base_diff, double foreign_diff);
    bool IsDescendant(Node* lhs, Node* rhs) const;

    QString CreatePath(const Node* node) const;
    void SortIterative(Node* node, const auto& Compare);
    void RecycleNode(NodeHash& node_hash);

private:
    Node* root_ {};

    const TreeInfo* info_ {};
    const SectionRule* section_rule_ {};
    const ViewHash* section_view_ {};
    const Interface* interface_ {};

    NodeHash node_hash_ {};
    StringHash leaf_path_ {};
    StringHash branch_path_ {};
    TreeSql* sql_ {};
};

#endif // TREEMODEL_H
