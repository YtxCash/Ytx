#include "checkstate.h"

#include <QApplication>
#include <QCheckBox>
#include <QMouseEvent>
#include <QPainter>

CheckState::CheckState(bool fill_rect, QObject* parent)
    : QStyledItemDelegate { parent }
    , rect_ { 0, 0, 16, 16 }
    , fill_rect_ { fill_rect }
{
}

QWidget* CheckState::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    return new QCheckBox(parent);
}

void CheckState::setEditorData(QWidget* editor, const QModelIndex& index) const { qobject_cast<QCheckBox*>(editor)->setChecked(index.data().toBool()); }

void CheckState::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(index);

    rect_.moveCenter(option.rect.center());
    editor->setGeometry(rect_);
}

void CheckState::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (fill_rect_ && (option.state & QStyle::State_Selected))
        painter->fillRect(option.rect, option.palette.highlight());

    rect_.moveCenter(option.rect.center());

    QStyleOptionButton check_box {};
    check_box.rect = rect_;
    check_box.state = index.data().toBool() ? QStyle::State_On : QStyle::State_Off;

    QApplication::style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &check_box, painter, option.widget);
}

bool CheckState::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if (event->type() == QEvent::MouseButtonPress && option.rect.contains(static_cast<QMouseEvent*>(event)->pos()))
        model->setData(index, !index.data().toBool());

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
