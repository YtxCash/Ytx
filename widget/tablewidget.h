#ifndef TABLEWIDGET_H
#define TABLEWIDGET_H

#include <QTableView>
#include <QWidget>

#include "table/abstracttablemodel.h"

namespace Ui {
class TableWidget;
}

class TableWidget final : public QWidget {
    Q_OBJECT

public:
    explicit TableWidget(QWidget* parent = nullptr);
    ~TableWidget();

    void SetModel(AbstractTableModel* model);

    AbstractTableModel* Model() { return model_; }
    QTableView* View();

private:
    Ui::TableWidget* ui;
    AbstractTableModel* model_ {};
};

#endif // TABLEWIDGET_H
