#include "treedoublespin.h"

#include <QPainter>

#include "widget/doublespinbox.h"

TreeDoubleSpin::TreeDoubleSpin(const int* decimal, double min, double max, QObject* parent)
    : QStyledItemDelegate { parent }
    , decimal_ { decimal }
    , max_ { max }
    , min_ { min }
    , locale_ { QLocale::English, QLocale::UnitedStates }
{
}

QWidget* TreeDoubleSpin::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    return new DoubleSpinBox(decimal_, min_, max_, parent);
}

void TreeDoubleSpin::setEditorData(QWidget* editor, const QModelIndex& index) const { qobject_cast<DoubleSpinBox*>(editor)->setValue(index.data().toDouble()); }

void TreeDoubleSpin::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(index);

    editor->setGeometry(option.rect);
}

void TreeDoubleSpin::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto cast_editor { qobject_cast<DoubleSpinBox*>(editor) };
    model->setData(index, cast_editor->cleanText().isEmpty() ? 0.0 : cast_editor->value());
}

void TreeDoubleSpin::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto value { index.data().toDouble() };
    if (value == 0)
        return QStyledItemDelegate::paint(painter, option, index);

    auto selected { option.state & QStyle::State_Selected };
    auto palette { option.palette };

    painter->setPen(selected ? palette.color(QPalette::HighlightedText) : palette.color(QPalette::Text));
    if (selected)
        painter->fillRect(option.rect, palette.highlight());

    painter->drawText(option.rect.adjusted(0, 0, -4, 0), Qt::AlignRight | Qt::AlignVCenter, locale_.toString(value, 'f', *decimal_));
}

QSize TreeDoubleSpin::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto value { index.data().toDouble() };
    return value == 0 ? QSize() : QSize(QFontMetrics(option.font).horizontalAdvance(locale_.toString(value, 'f', *decimal_)) + 8, option.rect.height());
}
