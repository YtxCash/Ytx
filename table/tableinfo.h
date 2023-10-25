#ifndef TABLEINFO_H
#define TABLEINFO_H

#include <QStringList>

#include "component/enumclass.h"

struct TableInfo {
    Section section {};
    QString transaction {}; // SQL database node transaction table name
    QStringList header {};
    QStringList search_header {};
};

#endif // TABLEINFO_H
