#include "tablevalue.h"

#include <QPainter>

#include "component/enumclass.h"
#include "widget/doublespinbox.h"

TableValue::TableValue(const int* decimal, QObject* parent)
    : QStyledItemDelegate { parent }
    , decimal_ { decimal }
    , locale_ { QLocale::English, QLocale::UnitedStates }
{
}

QWidget* TableValue::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    auto editor { new DoubleSpinBox(parent) };
    editor->setDecimals(*decimal_);

    return editor;
}

void TableValue::setEditorData(QWidget* editor, const QModelIndex& index) const { qobject_cast<DoubleSpinBox*>(editor)->setValue(index.data().toDouble()); }

void TableValue::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(index);

    editor->setGeometry(option.rect);
}

void TableValue::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto cast_editor { qobject_cast<DoubleSpinBox*>(editor) };
    model->setData(index, cast_editor->cleanText().isEmpty() ? 0.0 : cast_editor->value());
}

void TableValue::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto value { index.data().toDouble() };

    if (value == 0 && index.column() != static_cast<int>(TransColumn::kRemainder))
        return QStyledItemDelegate::paint(painter, option, index);

    painter->setPen(option.state & QStyle::State_Selected ? option.palette.color(QPalette::HighlightedText) : option.palette.color(QPalette::Text));
    painter->drawText(option.rect.adjusted(0, 0, -4, 0), Qt::AlignRight | Qt::AlignVCenter, locale_.toString(value, 'f', *decimal_));
}

QSize TableValue::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto value { index.data().toDouble() };
    return value == 0 ? QSize() : QSize(QFontMetrics(option.font).horizontalAdvance(locale_.toString(value, 'f', *decimal_)) + 8, option.rect.height());
}
