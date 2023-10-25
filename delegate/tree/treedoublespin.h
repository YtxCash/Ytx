#ifndef TREEDOUBLESPIN_H
#define TREEDOUBLESPIN_H

#include <QStyledItemDelegate>

class TreeDoubleSpin : public QStyledItemDelegate {
public:
    TreeDoubleSpin(const int* decimal, double min, double max, QObject* parent);
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    const int* decimal_ {};
    double max_ {};
    double min_ {};
    QLocale locale_ {};
};

#endif // TREEDOUBLESPIN_H
