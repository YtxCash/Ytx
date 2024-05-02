#ifndef SNODEIDR_H
#define SNODEIDR_H

// read only

#include <QStyledItemDelegate>

#include "component/using.h"

class SNodeIDR : public QStyledItemDelegate {
public:
    SNodeIDR(CStringHash* leaf_path, CStringHash* branch_path, QObject* parent = nullptr);
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    QString GetPath(const QModelIndex& index) const;

private:
    CStringHash* leaf_path_ {};
    CStringHash* branch_path_ {};
};

#endif // SNODEIDR_H
