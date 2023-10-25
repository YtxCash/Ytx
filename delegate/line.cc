#include "line.h"

#include "widget/lineedit.h"

Line::Line(QObject* parent)
    : QStyledItemDelegate { parent }
{
}

QWidget* Line::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    auto editor { new LineEdit(parent) };
    return editor;
}

void Line::setEditorData(QWidget* editor, const QModelIndex& index) const { qobject_cast<LineEdit*>(editor)->setText(index.data().toString()); }

void Line::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(index);

    editor->setGeometry(option.rect);
}

void Line::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    model->setData(index, qobject_cast<LineEdit*>(editor)->text());
}
