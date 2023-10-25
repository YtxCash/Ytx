#include "transpool.h"

TransPool& TransPool::Instance()
{
    static TransPool instance;
    return instance;
}

SPTrans TransPool::Allocate()
{
    QMutexLocker locker(&mutex_);

    if (remain_ <= kExpandThreshold) {
        ExpandCapacity(kSize);
        remain_ += kSize;
    }

    --remain_;
    return pool_.dequeue();
}

void TransPool::Recycle(SPTrans trans)
{
    if (!trans)
        return;

    QMutexLocker locker(&mutex_);

    if (remain_ >= kShrinkThreshold) {
        ShrinkCapacity(kSize);
        remain_ = remain_ - kSize;
    }

    trans.data()->Reset();
    pool_.enqueue(trans);
    ++remain_;
}

TransPool::TransPool()
{
    ExpandCapacity(kSize);
    remain_ = kSize;
}

void TransPool::ExpandCapacity(int size)
{
    for (int i = 0; i != size; ++i)
        pool_.enqueue(SPTrans(new Trans()));
}

void TransPool::ShrinkCapacity(int size)
{
    for (int i = 0; i != size; ++i)
        pool_.dequeue();
}

TransPool::~TransPool()
{
    QMutexLocker locker(&mutex_);
    pool_.clear();
}
