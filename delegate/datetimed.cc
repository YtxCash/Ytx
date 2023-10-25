#include "datetimed.h"

#include <QPainter>

#include "component/constvalue.h"
#include "widget/datetimeedit.h"

DateTimeD::DateTimeD(const QString* date_format, const bool* hide_time, bool record, QObject* parent)
    : QStyledItemDelegate { parent }
    , date_format_ { date_format }
    , hide_time_ { hide_time }
    , record_ { record }
    , time_pattern_ { R"((\s?\S{2}:\S{2}:\S{2}\s?))" }
{
}

QWidget* DateTimeD::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    auto format { *date_format_ };
    if (*hide_time_)
        format.remove(time_pattern_);

    return new DateTimeEdit(format, parent);
}

void DateTimeD::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto date_time { index.data().toDateTime() };
    if (!date_time.isValid())
        date_time = last_date_time_.isValid() ? last_date_time_.addSecs(1) : QDateTime::currentDateTime();

    qobject_cast<DateTimeEdit*>(editor)->setDateTime(date_time);
}

void DateTimeD::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(index);

    editor->setGeometry(option.rect);
}

void DateTimeD::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto date_time { qobject_cast<DateTimeEdit*>(editor)->dateTime() };
    if (record_)
        last_date_time_ = date_time.date() == QDate::currentDate() ? QDateTime() : date_time;

    model->setData(index, date_time.toString(DATE_TIME_FST));
}

void DateTimeD::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto date_time { index.data().toDateTime() };
    if (!date_time.isValid())
        return QStyledItemDelegate::paint(painter, option, index);

    painter->setPen(option.state & QStyle::State_Selected ? option.palette.color(QPalette::HighlightedText) : option.palette.color(QPalette::Text));
    painter->drawText(option.rect, Qt::AlignCenter, FormatDateTime(date_time));
}

QSize DateTimeD::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto date_time { index.data().toDateTime() };
    return date_time.isValid() ? QSize(QFontMetrics(option.font).horizontalAdvance(FormatDateTime(date_time)) + 8, option.rect.height()) : QSize();
}

QString DateTimeD::FormatDateTime(const QDateTime& date_time) const
{
    auto format { *date_format_ };
    if (*hide_time_)
        format.remove(time_pattern_);

    return date_time.toString(format);
}
