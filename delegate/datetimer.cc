#include "datetimer.h"

#include <QDateTime>
#include <QPainter>

DateTimeR::DateTimeR(const QString* date_format, const bool* hide_time, QObject* parent)
    : QStyledItemDelegate { parent }
    , date_format_ { date_format }
    , hide_time_ { hide_time }
    , time_pattern_ { R"((\s?\S{2}:\S{2}:\S{2}\s?))" }
{
}

void DateTimeR::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto date_time { index.data().toDateTime() };
    if (!date_time.isValid())
        return QStyledItemDelegate::paint(painter, option, index);

    painter->setPen(option.state & QStyle::State_Selected ? option.palette.color(QPalette::HighlightedText) : option.palette.color(QPalette::Text));
    painter->drawText(option.rect, Qt::AlignCenter, FormattedString(date_time));
}

QSize DateTimeR::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto date_time { index.data().toDateTime() };
    return date_time.isValid() ? QSize(QFontMetrics(option.font).horizontalAdvance(FormattedString(date_time)) + 8, option.rect.height()) : QSize();
}

QString DateTimeR::FormattedString(const QDateTime& date_time) const
{
    auto format { *date_format_ };
    if (*hide_time_)
        format.remove(time_pattern_);

    return date_time.toString(format);
}
