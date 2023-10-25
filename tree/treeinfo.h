#ifndef TREEINFO_H
#define TREEINFO_H

#include "component/enumclass.h"
#include "component/using.h"

struct TreeInfo {
    Section section {};
    QString node {}; // SQL database node table name, also used as QSettings section name, be carefull with it
    QStringList header {};

    StringHash unit_hash {};
    StringHash unit_symbol_hash {};
};

#endif // TREEINFO_H
