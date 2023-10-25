#ifndef TREECOMBO_H
#define TREECOMBO_H

#include <QStyledItemDelegate>

#include "component/using.h"

class TreeCombo final : public QStyledItemDelegate {
public:
    TreeCombo(CStringHash* hash, QObject* parent);
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    CStringHash* hash_ {};
};

#endif // TREECOMBO_H
