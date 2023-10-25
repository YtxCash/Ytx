#ifndef TREEWIDGETORDER_H
#define TREEWIDGETORDER_H

#include "component/info.h"
#include "component/settings.h"
#include "tree/abstracttreemodel.h"
#include "widget/abstracttreewidget.h"

namespace Ui {
class TreeWidgetOrder;
}

class TreeWidgetOrder final : public AbstractTreeWidget {
    Q_OBJECT

public:
    TreeWidgetOrder(AbstractTreeModel* model, const Info* info, const SectionRule* section_rule, QWidget* parent = nullptr);
    ~TreeWidgetOrder() override;

    void SetCurrentIndex(const QModelIndex& index) override;
    void ResetStatus() override;
    void HideStatus() override;

    QTreeView* View() override;
    QHeaderView* Header() override;

public slots:
    void RUpdateDSpinBox() override;

    void on_dateEditStart_dateChanged(const QDate& date);
    void on_dateEditEnd_dateChanged(const QDate& date);

private:
    void DynamicStatus(const Node* lhs, const Node* rhs);
    void StaticStatus(const Node* node);

    double Operate(double lhs, double rhs, const QString& operation);

private:
    Ui::TreeWidgetOrder* ui;

    AbstractTreeModel* model_ {};
    const Info* info_ {};
    const SectionRule* section_rule_ {};

    const Node* static_node_ {};
    const Node* dynamic_node_lhs_ {};
    const Node* dynamic_node_rhs_ {};
};

#endif // TREEWIDGETORDER_H
