#include "nodepool.h"

NodePool& NodePool::Instance()
{
    static NodePool instance;
    return instance;
}

Node* NodePool::Allocate()
{
    QMutexLocker locker(&mutex_);

    if (remain_ <= kExpandThreshold) {
        ExpandCapacity(kSize);
        remain_ += kSize;
    }

    --remain_;
    return pool_.dequeue();
}

void NodePool::Recycle(Node* node)
{
    if (!node)
        return;

    QMutexLocker locker(&mutex_);

    if (remain_ >= kShrinkThreshold) {
        ShrinkCapacity(kSize);
        remain_ -= kSize;
    }

    node->Reset();
    pool_.enqueue(node);
    ++remain_;
}

NodePool::NodePool()
{
    ExpandCapacity(kSize);
    remain_ = kSize;
}

NodePool::~NodePool()
{
    QMutexLocker locker(&mutex_);
    qDeleteAll(pool_);
    pool_.clear();
}

void NodePool::ExpandCapacity(int size)
{
    for (int i = 0; i != size; ++i)
        pool_.enqueue(new Node());
}

void NodePool::ShrinkCapacity(int size)
{
    for (int i = 0; i != size; ++i)
        delete pool_.dequeue();
}
