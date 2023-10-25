#include "document.h"

#include <QMouseEvent>

Document::Document(QObject* parent)
    : QStyledItemDelegate { parent }
{
}

void Document::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto opt { option };
    opt.displayAlignment = Qt::AlignCenter;
    QStyledItemDelegate::paint(painter, opt, index);
}

bool Document::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if (event->type() == QEvent::MouseButtonDblClick && option.rect.contains(static_cast<QMouseEvent*>(event)->pos()))
        emit SUpdateDocument();

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
