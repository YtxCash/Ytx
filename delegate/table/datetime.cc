#include "datetime.h"

#include <QPainter>

#include "widget/datetimeedit.h"

DateTime::DateTime(const QString* date_format, const bool* hide_time, QObject* parent)
    : QStyledItemDelegate { parent }
    , date_format_ { date_format }
    , hide_time_ { hide_time }
    , time_pattern_ { R"((\s?\S{2}:\S{2}:\S{2}\s?))" }
{
}

QWidget* DateTime::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    auto editor { new DateTimeEdit(parent) };

    auto format { *date_format_ };
    if (*hide_time_)
        format.remove(time_pattern_);

    editor->setDisplayFormat(format);
    return editor;
}

void DateTime::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto date_time { index.data().toDateTime() };
    if (!date_time.isValid())
        date_time = last_insert_.isValid() ? last_insert_.addSecs(1) : QDateTime::currentDateTime();

    qobject_cast<DateTimeEdit*>(editor)->setDateTime(date_time);
}

void DateTime::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(index);

    editor->setGeometry(option.rect);
}

void DateTime::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto date_time { qobject_cast<DateTimeEdit*>(editor)->dateTime() };
    last_insert_ = date_time.date() == QDate::currentDate() ? QDateTime() : date_time;

    model->setData(index, date_time.toString("yyyy-MM-dd hh:mm:ss"));
}

void DateTime::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto date_time { index.data().toDateTime() };
    if (!date_time.isValid())
        return QStyledItemDelegate::paint(painter, option, index);

    painter->setPen(option.state & QStyle::State_Selected ? option.palette.color(QPalette::HighlightedText) : option.palette.color(QPalette::Text));
    painter->drawText(option.rect.adjusted(4, 0, 0, 0), Qt::AlignLeft | Qt::AlignVCenter, FormattedString(date_time));
}

QSize DateTime::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto date_time { index.data().toDateTime() };
    return date_time.isValid() ? QSize(QFontMetrics(option.font).horizontalAdvance(FormattedString(date_time)) + 8, option.rect.height()) : QSize();
}

QString DateTime::FormattedString(const QDateTime& date_time) const
{
    auto format { *date_format_ };
    if (*hide_time_)
        format.remove(time_pattern_);

    return date_time.toString(format);
}
