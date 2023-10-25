#ifndef TRANSACTIONPOOL_H
#define TRANSACTIONPOOL_H

#include <QMutex>
#include <QQueue>
#include <QSharedPointer>

#include "table/transaction.h"

using SPTransaction = QSharedPointer<Transaction>;

class TransactionPool {
public:
    static TransactionPool& Instance();
    SPTransaction Allocate();
    void Recycle(SPTransaction transaction);

private:
    TransactionPool();
    ~TransactionPool();

    TransactionPool(const TransactionPool&) = delete;
    TransactionPool& operator=(const TransactionPool&) = delete;
    TransactionPool(TransactionPool&&) = delete;
    TransactionPool& operator=(TransactionPool&&) = delete;

    void ExpandCapacity(int size);
    void ShrinkCapacity(int size);

private:
    QQueue<SPTransaction> pool_ {};
    QMutex mutex_ {};

    const int kSize { 100 };
    const int kExpandThreshold { 20 };
    const int kShrinkThreshold { 400 };
    int remain_ { 0 };
};

#endif // TRANSACTIONPOOL_H
