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

    return new LineEdit(parent);
}

void Line::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto cast_editor { qobject_cast<LineEdit*>(editor) };
    if (cast_editor)
        cast_editor->setText(index.data().toString());
}

void Line::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(index);

    editor->setGeometry(option.rect);
}

void Line::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto cast_editor { qobject_cast<LineEdit*>(editor) };
    if (cast_editor)
        model->setData(index, cast_editor->text());
}
