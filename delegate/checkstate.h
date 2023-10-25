#ifndef CHECKSTATE_H
#define CHECKSTATE_H

// tree's branch column 6, table's state column 7, arte different, so they can share this delegate

#include <QStyledItemDelegate>

class CheckState final : public QStyledItemDelegate {
public:
    CheckState(bool fill_rect, QObject* parent = nullptr);
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;

private:
    mutable QRect rect_ {};
    bool fill_rect_ {};
};

#endif // CHECKSTATE_H
