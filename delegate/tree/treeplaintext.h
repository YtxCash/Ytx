#ifndef TREEPLAINTEXT_H
#define TREEPLAINTEXT_H

#include <QStyledItemDelegate>

class TreePlainText : public QStyledItemDelegate {
public:
    explicit TreePlainText(QObject* parent = nullptr);
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
};

#endif // TREEPLAINTEXT_H
