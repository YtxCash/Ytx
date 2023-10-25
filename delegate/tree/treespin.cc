#include "treespin.h"

#include <QPainter>

#include "widget/spinbox.h"

TreeSpin::TreeSpin(int min, int max, QObject* parent)
    : QStyledItemDelegate { parent }
    , max_ { max }
    , min_ { min }
    , locale_ { QLocale::English, QLocale::UnitedStates }
{
}

QWidget* TreeSpin::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    return new SpinBox(min_, max_, parent);
}

void TreeSpin::setEditorData(QWidget* editor, const QModelIndex& index) const { qobject_cast<SpinBox*>(editor)->setValue(index.data().toInt()); }

void TreeSpin::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(index);

    editor->setGeometry(option.rect);
}

void TreeSpin::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto cast_editor { qobject_cast<SpinBox*>(editor) };
    model->setData(index, cast_editor->cleanText().isEmpty() ? 0.0 : cast_editor->value());
}

void TreeSpin::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto value { index.data().toInt() };
    if (value == 0)
        return QStyledItemDelegate::paint(painter, option, index);

    auto selected { option.state & QStyle::State_Selected };
    auto palette { option.palette };

    painter->setPen(selected ? palette.color(QPalette::HighlightedText) : palette.color(QPalette::Text));
    if (selected)
        painter->fillRect(option.rect, palette.highlight());

    painter->drawText(option.rect.adjusted(0, 0, -4, 0), Qt::AlignRight | Qt::AlignVCenter, locale_.toString(value));
}

QSize TreeSpin::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto value { index.data().toInt() };
    return value == 0 ? QSize() : QSize(QFontMetrics(option.font).horizontalAdvance(locale_.toString(value)) + 8, option.rect.height());
}
