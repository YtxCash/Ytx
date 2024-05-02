#include "spinbox.h"

#include <QPainter>
#include <QSpinBox>

SpinBox::SpinBox(int min, int max, QObject* parent)
    : QStyledItemDelegate { parent }
    , max_ { max }
    , min_ { min }
{
}

QWidget* SpinBox::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    auto editor { new QSpinBox(parent) };
    editor->setStepType(QAbstractSpinBox::AdaptiveDecimalStepType);
    editor->setButtonSymbols(QAbstractSpinBox::NoButtons);
    editor->setAlignment(Qt::AlignCenter);
    editor->setRange(min_, max_);

    return editor;
}

void SpinBox::setEditorData(QWidget* editor, const QModelIndex& index) const { qobject_cast<QSpinBox*>(editor)->setValue(index.data().toInt()); }

void SpinBox::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(index);

    editor->setGeometry(option.rect);
}

void SpinBox::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto cast_editor { qobject_cast<QSpinBox*>(editor) };
    model->setData(index, cast_editor->cleanText().isEmpty() ? 0.0 : cast_editor->value());
}

void SpinBox::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto opt { option };
    opt.displayAlignment = Qt::AlignCenter;
    QStyledItemDelegate::paint(painter, opt, index);
}
