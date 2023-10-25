#ifndef SEARCHSQL_H
#define SEARCHSQL_H

#include <QSqlDatabase>

#include "component/enumclass.h"
#include "component/using.h"

class SearchSql {
public:
    SearchSql() = default;
    SearchSql(CString& node, CString& transaction, Section section, SPTransactionHash* transaction_hash);

    QList<int> Node(CString& text);
    SPTransactionList Trans(CString& text);

private:
    QSqlDatabase* db_ {};
    QString transaction_ {}; // SQL database node transaction table name
    QString node_ {}; // SQL database node table name, also used as QSettings section name, be carefull with it
    SPTransactionHash* transaction_hash_ {};
};

#endif // SEARCHSQL_H
