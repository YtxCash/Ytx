#ifndef SPINBOX_H
#define SPINBOX_H

#include <QStyledItemDelegate>

class SpinBox : public QStyledItemDelegate {
public:
    SpinBox(int min, int max, QObject* parent = nullptr);
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    int max_ {};
    int min_ {};
};

#endif // SPINBOX_H
