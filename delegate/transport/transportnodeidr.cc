#include "transportnodeidr.h"

#include <QPainter>

TransportNodeIDR::TransportNodeIDR(CSPCTrans trans, const QHash<Section, CStringHash*>* leaf_path_hash, QObject* parent)
    : QStyledItemDelegate { parent }
    , trans_ { trans }
    , leaf_path_hash_ { leaf_path_hash }
{
}

void TransportNodeIDR::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto path { GetPath(index) };
    if (path.isEmpty())
        return QStyledItemDelegate::paint(painter, option, index);

    painter->setPen(option.state & QStyle::State_Selected ? option.palette.color(QPalette::HighlightedText) : option.palette.color(QPalette::Text));
    painter->drawText(option.rect.adjusted(4, 0, 0, 0), Qt::AlignLeft | Qt::AlignVCenter, path);
}

QSize TransportNodeIDR::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto path { GetPath(index) };
    return path.isEmpty() ? QSize() : QSize(QFontMetrics(option.font).horizontalAdvance(path) + 8, option.rect.height());
}

QString TransportNodeIDR::GetPath(const QModelIndex& index) const
{
    return leaf_path_hash_->value(Section { trans_->location->at(index.row() * 2).toInt() })->value(index.data().toInt());
}
