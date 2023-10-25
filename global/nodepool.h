#ifndef NODEPOOL_H
#define NODEPOOL_H

#include <QMutex>
#include <QQueue>

#include "tree/node.h"

class NodePool {
public:
    static NodePool& Instance();
    Node* Allocate();
    void Recycle(Node* node);

private:
    NodePool();
    ~NodePool();

    NodePool(const NodePool&) = delete;
    NodePool& operator=(const NodePool&) = delete;
    NodePool(NodePool&&) = delete;
    NodePool& operator=(NodePool&&) = delete;

    void ExpandCapacity(int size);
    void ShrinkCapacity(int size);

private:
    QQueue<Node*> pool_ {};
    QMutex mutex_ {};

    const int kSize { 50 };
    const int kExpandThreshold { 10 };
    const int kShrinkThreshold { 200 };
    int remain_ { 0 };
};

#endif // NODEPOOL_H
