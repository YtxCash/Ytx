#ifndef ABSTRACTTREEWIDGET_H
#define ABSTRACTTREEWIDGET_H

#include <QTreeView>
#include <QWidget>

class AbstractTreeWidget : public QWidget {
    Q_OBJECT

public:
    AbstractTreeWidget(QWidget* parent = nullptr)
        : QWidget { parent }
    {
    }
    virtual ~AbstractTreeWidget() = default;

    virtual void SetCurrentIndex(const QModelIndex& index) = 0;
    virtual void ResetStatus() = 0;
    virtual void HideStatus() = 0;

    virtual QTreeView* View() = 0;
    virtual QHeaderView* Header() = 0;

public slots:
    virtual void RUpdateDSpinBox() = 0;
};

#endif // ABSTRACTTREEWIDGET_H
