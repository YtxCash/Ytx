#ifndef TABLEDATETIME_H
#define TABLEDATETIME_H

#include <QDateTimeEdit>
#include <QRegularExpression>
#include <QStyledItemDelegate>

class TableDateTime : public QStyledItemDelegate {
public:
    TableDateTime(const QString* date_format, const bool* hide_time, QObject* parent = nullptr);
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    QString FormattedString(const QDateTime& date_time) const;

private:
    const QString* date_format_ {};
    const bool* hide_time_ {};
    QRegularExpression time_pattern_ {};
    mutable QDateTime last_insert_ {};
};

#endif // TABLEDATETIME_H
