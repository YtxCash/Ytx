#include "treedoublespinpercent.h"

#include <QPainter>

#include "component/constvalue.h"
#include "widget/doublespinbox.h"

TreeDoubleSpinPercent::TreeDoubleSpinPercent(const int* decimal, double min, double max, QObject* parent)
    : QStyledItemDelegate { parent }
    , decimal_ { decimal }
    , max_ { max }
    , min_ { min }
    , locale_ { QLocale::English, QLocale::UnitedStates }
{
}

QWidget* TreeDoubleSpinPercent::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    auto editor { new DoubleSpinBox(decimal_, min_, max_, parent) };
    editor->setSuffix(SFX_PERCENT);

    return editor;
}

void TreeDoubleSpinPercent::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    qobject_cast<DoubleSpinBox*>(editor)->setValue(index.data().toDouble() * HUNDRED);
}

void TreeDoubleSpinPercent::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(index);

    editor->setGeometry(option.rect);
}

void TreeDoubleSpinPercent::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto cast_editor { qobject_cast<DoubleSpinBox*>(editor) };
    model->setData(index, (cast_editor->cleanText().isEmpty() ? 0.0 : cast_editor->value()) / HUNDRED);
}

void TreeDoubleSpinPercent::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto value { index.data().toDouble() * HUNDRED };
    if (value == 0)
        return QStyledItemDelegate::paint(painter, option, index);

    auto selected { option.state & QStyle::State_Selected };
    auto palette { option.palette };

    painter->setPen(selected ? palette.color(QPalette::HighlightedText) : palette.color(QPalette::Text));
    if (selected)
        painter->fillRect(option.rect, palette.highlight());

    painter->drawText(option.rect.adjusted(0, 0, -4, 0), Qt::AlignRight | Qt::AlignVCenter, locale_.toString(value, 'f', *decimal_) + SFX_PERCENT);
}

QSize TreeDoubleSpinPercent::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto value { index.data().toDouble() * HUNDRED };
    return value == 0 ? QSize()
                      : QSize(QFontMetrics(option.font).horizontalAdvance(locale_.toString(value, 'f', *decimal_) + SFX_PERCENT) + 8, option.rect.height());
}
