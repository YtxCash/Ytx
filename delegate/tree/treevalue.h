#ifndef TREEVALUE_H
#define TREEVALUE_H

#include <QLocale>
#include <QStyledItemDelegate>

#include "component/using.h"

class TreeValue : public QStyledItemDelegate {
public:
    TreeValue(const int* decimal, CStringHash* unit_symbol_hash, QObject* parent = nullptr);
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    QString FormattedString(const QModelIndex& index) const;

private:
    const int* decimal_ {};
    CStringHash* unit_symbol_hash_ {};
    QLocale locale_ {};
};

#endif // TREEVALUE_H
