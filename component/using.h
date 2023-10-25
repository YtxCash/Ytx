#ifndef USING_H
#define USING_H

#include <QHash>
#include <QList>
#include <QSharedPointer>
#include <QString>
#include <QTableView>

#include "table/transaction.h"
#include "tree/node.h"

using StringHash = QHash<int, QString>;
using CStringHash = const QHash<int, QString>;
using NodeHash = QHash<int, Node*>;
using ViewHash = QHash<int, QTableView*>;

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
using CStringList = const QStringList;

#endif // USING_H
