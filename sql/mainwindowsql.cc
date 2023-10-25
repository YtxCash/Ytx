#include "mainwindowsql.h"

#include <QSqlError>
#include <QSqlQuery>

#include "component/constvalue.h"
#include "global/sqlconnection.h"

MainwindowSql::MainwindowSql(Section section)
    : db_ { SqlConnection::Instance().Allocate(section) }
{
}

void MainwindowSql::QuerySectionRule(SectionRule& section_rule, Section section)
{
    QSqlQuery query(*db_);
    query.setForwardOnly(true);

    auto part { "SELECT static_label, static_node, dynamic_label, dynamic_node_lhs, operation, dynamic_node_rhs, hide_time, base_unit, document_dir, "
                "value_decimal, ratio_decimal "
                "FROM section_rule "
                "WHERE id = :section " };

    query.prepare(part);
    query.bindValue(":section", static_cast<int>(section) + 1);
    if (!query.exec()) {
        qWarning() << "Failed to query section rule: " << query.lastError().text();
        return;
    }

    while (query.next()) {
        section_rule.static_label = query.value("static_label").toString();
        section_rule.static_node = query.value("static_node").toInt();
        section_rule.dynamic_label = query.value("dynamic_label").toString();
        section_rule.dynamic_node_lhs = query.value("dynamic_node_lhs").toInt();
        section_rule.operation = query.value("operation").toString();
        section_rule.dynamic_node_rhs = query.value("dynamic_node_rhs").toInt();
        section_rule.hide_time = query.value("hide_time").toBool();
        section_rule.base_unit = query.value("base_unit").toInt();
        section_rule.document_dir = query.value("document_dir").toString();
        section_rule.value_decimal = query.value("value_decimal").toInt();
        section_rule.ratio_decimal = query.value("ratio_decimal").toInt();
    }
}

void MainwindowSql::UpdateSectionRule(const SectionRule& section_rule, Section section)
{
    auto part {
        "UPDATE section_rule "
        "SET static_label = :static_label, static_node = :static_node, dynamic_label = :dynamic_label, dynamic_node_lhs = :dynamic_node_lhs, operation = "
        ":operation, dynamic_node_rhs = :dynamic_node_rhs, hide_time = :hide_time, base_unit = :base_unit, document_dir = :document_dir, value_decimal = "
        ":value_decimal, ratio_decimal = :ratio_decimal "
        "WHERE id = :section "
    };

    QSqlQuery query(*db_);

    query.prepare(part);
    query.bindValue(":section", static_cast<int>(section) + 1);
    query.bindValue(":static_label", section_rule.static_label);
    query.bindValue(":static_node", section_rule.static_node);
    query.bindValue(":dynamic_label", section_rule.dynamic_label);
    query.bindValue(":dynamic_node_lhs", section_rule.dynamic_node_lhs);
    query.bindValue(":operation", section_rule.operation);
    query.bindValue(":dynamic_node_rhs", section_rule.dynamic_node_rhs);
    query.bindValue(":hide_time", section_rule.hide_time);
    query.bindValue(":base_unit", section_rule.base_unit);
    query.bindValue(":document_dir", section_rule.document_dir);
    query.bindValue(":value_decimal", section_rule.value_decimal);
    query.bindValue(":ratio_decimal", section_rule.ratio_decimal);

    if (!query.exec()) {
        qWarning() << "Failed to update section rule: " << query.lastError().text();
        return;
    }
}

void MainwindowSql::NewFile(CString& file_path)
{
    QSqlDatabase db { QSqlDatabase::addDatabase(QSQLITE) };
    db.setDatabaseName(file_path);
    if (!db.open())
        return;

    QString finance = Node(FINANCE);
    QString finance_path = Path(FINANCE_PATH);
    QString finance_transaction = Transaction(FINANCE_TRANSACTION);

    QString product = Node(PRODUCT);
    QString product_path = Path(PRODUCT_PATH);
    QString product_transaction = Transaction(PRODUCT_TRANSACTION);

    QString task = Node(TASK);
    QString task_path = Path(TASK_PATH);
    QString task_transaction = Transaction(TASK_TRANSACTION);

    QString network = Node(NETWORK);
    QString network_path = Path(NETWORK_PATH);
    QString network_transaction = Transaction(NETWORK_TRANSACTION);

    QString section_rule = "CREATE TABLE IF NOT EXISTS section_rule ( \n"
                           "    id INTEGER PRIMARY KEY AUTOINCREMENT, \n"
                           "    static_label        TEXT, \n"
                           "    static_node         INTEGER, \n"
                           "    dynamic_label       TEXT, \n"
                           "    dynamic_node_lhs    INTEGER, \n"
                           "    operation           TEXT, \n"
                           "    dynamic_node_rhs    INTEGER, \n"
                           "    hide_time           BOOLEAN    DEFAULT TRUE, \n"
                           "    base_unit           INTEGER, \n"
                           "    document_dir        TEXT, \n"
                           "    value_decimal       INTEGER    DEFAULT 2, \n"
                           "    ratio_decimal       INTEGER    DEFAULT 4 \n"
                           ");";

    QString section_rule_row = "INSERT INTO section_rule (static_node) VALUES (0);";

    QSqlQuery query {};
    if (db.transaction()) {
        // Execute each table creation query
        if (query.exec(finance) && query.exec(finance_path) && query.exec(finance_transaction) && query.exec(product) && query.exec(product_path)
            && query.exec(product_transaction) && query.exec(network) && query.exec(network_path) && query.exec(network_transaction) && query.exec(task)
            && query.exec(task_path) && query.exec(task_transaction) && query.exec(section_rule)) {
            // Commit the transaction if all queries are successful
            if (db.commit()) {
                for (int i = 0; i != 6; ++i) {
                    query.exec(section_rule_row);
                }
            } else {
                // Handle commit failure
                qDebug() << "Error committing transaction:" << db.lastError().text();
                // Rollback the transaction in case of failure
                db.rollback();
            }
        } else {
            // Handle query execution failure
            qDebug() << "Error creating tables:" << query.lastError().text();
            // Rollback the transaction in case of failure
            db.rollback();
        }
    } else {
        // Handle transaction start failure
        qDebug() << "Error starting transaction:" << db.lastError().text();
    }

    db.close();
}

QString MainwindowSql::Node(CString& table_name)
{
    return QString("CREATE TABLE IF NOT EXISTS %1 ( \n"
                   "    id INTEGER PRIMARY KEY AUTOINCREMENT, \n"
                   "    name           TEXT, \n"
                   "    code           TEXT, \n"
                   "    description    TEXT, \n"
                   "    note           TEXT, \n"
                   "    node_rule      BOOLEAN    DEFAULT 0,  \n"
                   "    branch         BOOLEAN    DEFAULT 0,  \n"
                   "    unit           INTEGER, \n"
                   "    removed        BOOLEAN    DEFAULT 0   \n"
                   ");")
        .arg(table_name);
}

QString MainwindowSql::Path(CString& table_name)
{
    return QString("CREATE TABLE IF NOT EXISTS %1 ( \n"
                   "    ancestor      INTEGER  CHECK (ancestor   >= 1), \n"
                   "    descendant    INTEGER  CHECK (descendant >= 1), \n"
                   "    distance      INTEGER  CHECK (distance   >= 0)  \n"
                   ");")
        .arg(table_name);
}

QString MainwindowSql::Transaction(CString& table_name)
{
    return QString("CREATE TABLE IF NOT EXISTS %1 ( \n"
                   "    id INTEGER PRIMARY KEY AUTOINCREMENT, \n"
                   "    date_time      DATE,    \n"
                   "    code           TEXT,    \n"
                   "    lhs_node       INTEGER, \n"
                   "    lhs_ratio      REAL       DEFAULT 1.0  CHECK (lhs_ratio  > 0), \n"
                   "    lhs_debit      NUMERIC                 CHECK (lhs_debit  >=0), \n"
                   "    lhs_credit     NUMERIC                 CHECK (lhs_credit >=0), \n"
                   "    description    TEXT,    \n"
                   "    rhs_node       INTEGER, \n"
                   "    rhs_ratio      REAL       DEFAULT 1.0  CHECK (rhs_ratio  > 0), \n"
                   "    rhs_debit      NUMERIC                 CHECK (rhs_debit  >=0), \n"
                   "    rhs_credit     NUMERIC                 CHECK (rhs_credit >=0), \n"
                   "    state          BOOLEAN    DEFAULT 0,  \n"
                   "    document       TEXT,    \n"
                   "    removed        BOOLEAN    DEFAULT 0   \n"
                   ");")
        .arg(table_name);
}
