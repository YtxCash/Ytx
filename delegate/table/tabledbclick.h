#ifndef TABLEDBCLICK_H
#define TABLEDBCLICK_H

#include <QStyledItemDelegate>

class TableDbClick : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit TableDbClick(QObject* parent = nullptr);
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;

signals:
    void SEdit();
};

#endif // TABLEDBCLICK_H
