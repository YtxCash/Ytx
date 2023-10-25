#ifndef TABLECOMBO_H
#define TABLECOMBO_H

#include <QStyledItemDelegate>

#include "component/using.h"

class TableCombo final : public QStyledItemDelegate {
public:
    TableCombo(CStringHash* string_hash, int exclude, QObject* parent = nullptr);
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    CStringHash* string_hash_ {};
    int exclude_ {};
    mutable int last_insert_ {};
};

#endif // TABLECOMBO_H
