#ifndef REMOVENODE_H
#define REMOVENODE_H

#include <QDialog>

#include "component/using.h"

namespace Ui {
class RemoveNode;
}

class RemoveNode : public QDialog {
    Q_OBJECT

public:
    RemoveNode(CStringHash* leaf_path, int node_id, QWidget* parent = nullptr);
    ~RemoveNode();

signals:
    // send to tree model
    void SRemoveMulti(int node_id);
    void SReplaceMulti(int old_node_id, int new_node_id);

private slots:
    void RCustomAccept();

private:
    Ui::RemoveNode* ui;
    void IniDialog();
    void IniConnect();

private:
    int node_id_ {};
    int section_ {};

    CStringHash* leaf_path_ {};
};

#endif // REMOVENODE_H
