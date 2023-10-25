#ifndef TABLEDOUBLESPIN_H
#define TABLEDOUBLESPIN_H

#include <QLocale>
#include <QStyledItemDelegate>

class TableDoubleSpin final : public QStyledItemDelegate {
public:
    TableDoubleSpin(const int* decimal, double min, double max, QObject* parent = nullptr);
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

#endif // TABLEDOUBLESPIN_H
