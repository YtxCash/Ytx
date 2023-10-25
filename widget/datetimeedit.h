#ifndef DATETIMEEDIT_H
#define DATETIMEEDIT_H

#include <QDateTimeEdit>

class DateTimeEdit final : public QDateTimeEdit {
    Q_OBJECT

public:
    DateTimeEdit(const QString& format, QWidget* parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    void LastMonthEnd(QDateTime& date_time);
    void NextMonthStart(QDateTime& date_time);
};

#endif // DATETIMEEDIT_H
