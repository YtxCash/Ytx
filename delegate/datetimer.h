#ifndef DATETIMER_H
#define DATETIMER_H

#include <QStyledItemDelegate>

class DateTimeR : public QStyledItemDelegate {
public:
    DateTimeR(const QString* date_format, const bool* hide_time, QObject* parent = nullptr);
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    QString FormattedString(const QDateTime& date_time) const;

private:
    const QString* date_format_ {};
    const bool* hide_time_ {};
    QRegularExpression time_pattern_ {};
};

#endif // DATETIMER_H
