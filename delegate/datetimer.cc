#include "datetimer.h"

#include <QDateTime>
#include <QPainter>

DateTimeR::DateTimeR(const bool* hide_time, QObject* parent)
    : QStyledItemDelegate { parent }
    , hide_time_ { hide_time }
    , time_pattern_ { R"((\s?\S{2}:\S{2}:\S{2}\s?))" }
{
}

void DateTimeR::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto string { index.data().toString() };
    if (string.isEmpty())
        return QStyledItemDelegate::paint(painter, option, index);

    painter->setPen(option.state & QStyle::State_Selected ? option.palette.color(QPalette::HighlightedText) : option.palette.color(QPalette::Text));
    painter->drawText(option.rect, Qt::AlignCenter, Format(string));
}

QSize DateTimeR::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto string { index.data().toString() };
    return string.isEmpty() ? QSize() : QSize(QFontMetrics(option.font).horizontalAdvance(Format(string)) + 8, option.rect.height());
}

QString DateTimeR::Format(QString& string) const { return *hide_time_ ? string.remove(time_pattern_) : string; }
