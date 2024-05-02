#ifndef INFO_H
#define INFO_H

#include "enumclass.h"
#include "using.h"

struct Info {
    Section section {};

    QString node {}; // SQL database node table name, also used as QSettings section name, be carefull with it
    QString path {}; // SQL database node_path table name
    QString transaction {}; // SQL database node_transaction table name

    QStringList tree_header {};
    QStringList partial_table_header {};
    QStringList table_header {};

    StringHash unit_hash {};
    StringHash unit_symbol_hash {};
};

#endif // INFO_H
