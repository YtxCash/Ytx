#ifndef TRANSPORTNODEIDR_H
#define TRANSPORTNODEIDR_H

// read only

#include <QStyledItemDelegate>

#include "component/enumclass.h"
#include "component/using.h"

class TransportNodeIDR final : public QStyledItemDelegate {
public:
    TransportNodeIDR(CSPCTrans trans, const QHash<Section, CStringHash*>* leaf_path_hash, QObject* parent = nullptr);
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    QString GetPath(const QModelIndex& index) const;

private:
    CSPCTrans trans_ {};
    const QHash<Section, CStringHash*>* leaf_path_hash_ {};
};

#endif // TRANSPORTNODEIDR_H
