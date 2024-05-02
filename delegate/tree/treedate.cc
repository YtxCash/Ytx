#include "treedate.h"

#include <QPainter>

#include "component/constvalue.h"
#include "widget/datetimeedit.h"

TreeDate::TreeDate(QObject* parent)
    : QStyledItemDelegate { parent }
{
}

QWidget* TreeDate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    auto editor { new DateTimeEdit(parent) };
    editor->setDisplayFormat(DATE_FST);
    return editor;
}

void TreeDate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto date { index.data().toDate() };
    if (!date.isValid())
        date = last_insert_.isValid() ? last_insert_ : QDate::currentDate();

    qobject_cast<DateTimeEdit*>(editor)->setDate(date);
}

void TreeDate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(index);

    editor->setGeometry(option.rect);
}

void TreeDate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto date { qobject_cast<DateTimeEdit*>(editor)->date() };
    last_insert_ = date;

    model->setData(index, date.toString(DATE_FST));
}

void TreeDate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto string { index.data().toString() };
    if (string.isEmpty())
        return QStyledItemDelegate::paint(painter, option, index);

    auto selected { option.state & QStyle::State_Selected };
    auto palette { option.palette };

    painter->setPen(selected ? palette.color(QPalette::HighlightedText) : palette.color(QPalette::Text));
    if (selected)
        painter->fillRect(option.rect, palette.highlight());

    painter->drawText(option.rect.adjusted(4, 0, 0, 0), Qt::AlignCenter, string);
}

QSize TreeDate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto string { index.data().toString() };
    return string.isEmpty() ? QSize() : QSize(QFontMetrics(option.font).horizontalAdvance(string) + 8, option.rect.height());
}
