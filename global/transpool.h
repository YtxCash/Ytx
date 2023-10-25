#ifndef TRANSPOOL_H
#define TRANSPOOL_H

#include <QMutex>
#include <QQueue>
#include <QSharedPointer>

#include "table/transaction.h"

using SPTrans = QSharedPointer<Trans>;

class TransPool {
public:
    static TransPool& Instance();
    SPTrans Allocate();
    void Recycle(SPTrans trans);

private:
    TransPool();
    ~TransPool();

    TransPool(const TransPool&) = delete;
    TransPool& operator=(const TransPool&) = delete;
    TransPool(TransPool&&) = delete;
    TransPool& operator=(TransPool&&) = delete;

    void ExpandCapacity(int size);
    void ShrinkCapacity(int size);

private:
    QQueue<SPTrans> pool_ {};
    QMutex mutex_ {};

    const int kSize { 100 };
    const int kExpandThreshold { 20 };
    const int kShrinkThreshold { 400 };
    int remain_ { 0 };
};

#endif // TRANSPOOL_H
