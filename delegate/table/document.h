#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QStyledItemDelegate>

class Document : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit Document(QObject* parent = nullptr);
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;

signals:
    void SUpdateDocument();
};

#endif // DOCUMENT_H
