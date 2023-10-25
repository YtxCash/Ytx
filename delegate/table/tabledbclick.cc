#include "tabledbclick.h"

#include <QMouseEvent>

TableDbClick::TableDbClick(QObject* parent)
    : QStyledItemDelegate { parent }
{
}

void TableDbClick::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto opt { option };
    opt.displayAlignment = Qt::AlignCenter;
    QStyledItemDelegate::paint(painter, opt, index);
}

bool TableDbClick::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if (event->type() == QEvent::MouseButtonDblClick && option.rect.contains(static_cast<QMouseEvent*>(event)->pos()))
        emit SEdit();

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
