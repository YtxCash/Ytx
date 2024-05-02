#ifndef TREEVALUER_H
#define TREEVALUER_H

// read only

#include <QLocale>
#include <QStyledItemDelegate>

#include "component/using.h"

class TreeValueR : public QStyledItemDelegate {
public:
    TreeValueR(const int* decimal, CStringHash* unit_symbol_hash, QObject* parent = nullptr);
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    QString FormattedString(const QModelIndex& index) const;

private:
    const int* decimal_ {};
    CStringHash* unit_symbol_hash_ {};
    QLocale locale_ {};
};

#endif // TREEVALUER_H
