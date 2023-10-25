#ifndef MAINWINDOWSQL_H
#define MAINWINDOWSQL_H

#include <QSqlDatabase>

#include "component/enumclass.h"
#include "component/settings.h"
#include "component/using.h"

class MainwindowSql {
public:
    MainwindowSql() = default;
    explicit MainwindowSql(Section section);

    void QuerySectionRule(SectionRule& section_rule, Section section);
    void UpdateSectionRule(const SectionRule& section_rule, Section section);
    void NewFile(CString& file_path);

private:
    QString Node(CString& table_name);
    QString Path(CString& table_name);
    QString Transaction(CString& table_name);

private:
    QSqlDatabase* db_ {};
};

#endif // MAINWINDOWSQL_H
