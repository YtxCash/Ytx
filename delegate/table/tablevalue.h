#ifndef TABLEVALUE_H
#define TABLEVALUE_H

#include <QLocale>
#include <QStyledItemDelegate>

class TableValue : public QStyledItemDelegate {
public:
    TableValue(const int* decimal, QObject* parent = nullptr);
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    const int* decimal_ {};
    QLocale locale_ {};
};

#endif // TABLEVALUE_H
