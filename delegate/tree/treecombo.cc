#include "treecombo.h"

#include <widget/combobox.h>

#include <QPainter>

TreeCombo::TreeCombo(CStringHash* hash, QObject* parent)
    : QStyledItemDelegate { parent }
    , hash_ { hash }
{
}

QWidget* TreeCombo::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    auto editor { new ComboBox(parent) };

    for (auto it = hash_->cbegin(); it != hash_->cend(); ++it)
        editor->addItem(it.value(), it.key());

    editor->model()->sort(0);
    return editor;
}

void TreeCombo::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto cast_editor { qobject_cast<ComboBox*>(editor) };

    int item_index { cast_editor->findData(index.data().toInt()) };
    if (item_index != -1)
        cast_editor->setCurrentIndex(item_index);
}

void TreeCombo::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(index);

    editor->setGeometry(option.rect);
}

void TreeCombo::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    model->setData(index, qobject_cast<ComboBox*>(editor)->currentData().toInt());
}

void TreeCombo::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QString path { hash_->value(index.data().toInt()) };

    auto selected { option.state & QStyle::State_Selected };
    auto palette { option.palette };

    painter->setPen(selected ? palette.color(QPalette::HighlightedText) : palette.color(QPalette::Text));
    if (selected)
        painter->fillRect(option.rect, palette.highlight());

    painter->drawText(option.rect, Qt::AlignCenter, path);
}

QSize TreeCombo::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QString path { hash_->value(index.data().toInt()) };
    return path.isEmpty() ? QSize() : QSize(QFontMetrics(option.font).horizontalAdvance(path) + 8, option.rect.height());
}
