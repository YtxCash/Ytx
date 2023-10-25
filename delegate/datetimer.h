#ifndef DATETIMER_H
#define DATETIMER_H

#include <QStyledItemDelegate>

class DateTimeR final : public QStyledItemDelegate {
public:
    DateTimeR(const bool* hide_time, QObject* parent = nullptr);
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    QString Format(QString& date_time) const;

private:
    const bool* hide_time_ {};
    QRegularExpression time_pattern_ {};
};

#endif // DATETIMER_H
