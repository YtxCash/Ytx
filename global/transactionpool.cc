#include "transactionpool.h"

TransactionPool& TransactionPool::Instance()
{
    static TransactionPool instance;
    return instance;
}

SPTransaction TransactionPool::Allocate()
{
    QMutexLocker locker(&mutex_);

    if (remain_ <= kExpandThreshold) {
        ExpandCapacity(kSize);
        remain_ += kSize;
    }

    --remain_;
    return pool_.dequeue();
}

void TransactionPool::Recycle(SPTransaction transaction)
{
    if (!transaction)
        return;

    QMutexLocker locker(&mutex_);

    if (remain_ >= kShrinkThreshold) {
        ShrinkCapacity(kSize);
        remain_ = remain_ - kSize;
    }

    transaction.data()->Reset();
    pool_.enqueue(transaction);
    ++remain_;
}

TransactionPool::TransactionPool()
{
    ExpandCapacity(kSize);
    remain_ = kSize;
}

void TransactionPool::ExpandCapacity(int size)
{
    for (int i = 0; i != size; ++i)
        pool_.enqueue(SPTransaction(new Transaction()));
}

void TransactionPool::ShrinkCapacity(int size)
{
    for (int i = 0; i != size; ++i)
        pool_.dequeue();
}

TransactionPool::~TransactionPool()
{
    QMutexLocker locker(&mutex_);
    pool_.clear();
}
