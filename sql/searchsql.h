#ifndef SEARCHSQL_H
#define SEARCHSQL_H

#include <QSqlDatabase>

#include "component/info.h"
#include "component/using.h"

class SearchSql {
public:
    SearchSql() = default;
    SearchSql(const Info* info, SPTransactionHash* transaction_hash);

    QList<int> Node(CString& text);
    SPTransactionList Trans(CString& text);

private:
    QSqlDatabase* db_ {};
    SPTransactionHash* transaction_hash_ {};
    const Info* info_ {};
};

#endif // SEARCHSQL_H
