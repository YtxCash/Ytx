#ifndef NODEID_H
#define NODEID_H

#include <QStyledItemDelegate>

#include "component/using.h"

class NodeID : public QStyledItemDelegate {
public:
    NodeID(CStringHash* leaf_path, int node_id, QObject* parent = nullptr);
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    CStringHash* leaf_path_ {};
    int node_id_ {};
    mutable int last_insert_ {};
};

#endif // NODEID_H
