#include "tablewidget.h"

#include "ui_tablewidget.h"

TableWidget::TableWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::TableWidget)
{
    ui->setupUi(this);
}

TableWidget::~TableWidget() { delete ui; }

void TableWidget::SetModel(AbstractTableModel* model)
{
    ui->view_->setModel(model);
    model_ = model;
}

QTableView* TableWidget::View() { return ui->view_; }
