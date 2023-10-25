#ifndef COMBOHASH_H
#define COMBOHASH_H

#include <QStyledItemDelegate>

#include "component/using.h"

class ComboHash : public QStyledItemDelegate {
public:
    ComboHash(CStringHash* hash, QObject* parent = nullptr);
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    CStringHash* hash_ {};
};

#endif // COMBOHASH_H
