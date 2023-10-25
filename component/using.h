#ifndef USING_H
#define USING_H

#include <QHash>
#include <QSharedPointer>
#include <QStringList>

#include "table/transaction.h"
#include "tree/node.h"

using CStringHash = const QHash<int, QString>;
using NodeHash = QHash<int, Node*>;

using SPTransaction = QSharedPointer<Transaction>;
using SPTransactionList = QList<QSharedPointer<Transaction>>;
using SPTransactionHash = QHash<int, QSharedPointer<Transaction>>;
using CSPTransaction = const QSharedPointer<Transaction>;
using CSPCTransaction = const QSharedPointer<const Transaction>;

using SPTrans = QSharedPointer<Trans>;
using SPTransList = QList<QSharedPointer<Trans>>;
using CSPTrans = const QSharedPointer<Trans>;
using CSPCTrans = const QSharedPointer<const Trans>;

using CString = const QString;
using CVariant = const QVariant;
using CStringList = const QStringList;

#endif // USING_H
