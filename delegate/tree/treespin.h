#ifndef TREESPIN_H
#define TREESPIN_H

#include <QStyledItemDelegate>

class TreeSpin final : public QStyledItemDelegate {
public:
    TreeSpin(int min, int max, QObject* parent);
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    int max_ {};
    int min_ {};
    QLocale locale_ {};
};

#endif // TREESPIN_H
