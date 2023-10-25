#ifndef PLAINTEXT_H
#define PLAINTEXT_H

#include <QStyledItemDelegate>

class PlainText : public QStyledItemDelegate {
public:
    explicit PlainText(QObject* parent = nullptr);
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
};

#endif // PLAINTEXT_H
