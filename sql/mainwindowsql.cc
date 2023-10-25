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
    query.bindValue(":section", std::to_underlying(section) + 1);
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
    query.bindValue(":section", std::to_underlying(section) + 1);
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

    QString finance = NodeFinance(FINANCE);
    QString finance_path = Path(FINANCE_PATH);
    QString finance_transaction = TransactionFinance(FINANCE_TRANSACTION);

    QString product = NodeProduct(PRODUCT);
    QString product_path = Path(PRODUCT_PATH);
    QString product_transaction = TransactionProduct(PRODUCT_TRANSACTION);

    QString task = NodeTask(TASK);
    QString task_path = Path(TASK_PATH);
    QString task_transaction = TransactionTask(TASK_TRANSACTION);

    QString stakeholder = NodeStakeholder(STAKEHOLDER);
    QString stakeholder_path = Path(STAKEHOLDER_PATH);
    QString stakeholder_transaction = TransactionStakeholder(STAKEHOLDER_TRANSACTION);

    QString purchase = NodeOrder(PURCHASE);
    QString purchase_path = Path(PURCHASE_PATH);
    QString purchase_transaction = TransactionOrder(PURCHASE_TRANSACTION);

    QString sales = NodeOrder(SALES);
    QString sales_path = Path(SALES_PATH);
    QString sales_transaction = TransactionOrder(SALES_TRANSACTION);

    QString section_rule = "CREATE TABLE IF NOT EXISTS section_rule (      \n"
                           "  id INTEGER PRIMARY KEY AUTOINCREMENT,        \n"
                           "  static_label        TEXT,                    \n"
                           "  static_node         INTEGER,                 \n"
                           "  dynamic_label       TEXT,                    \n"
                           "  dynamic_node_lhs    INTEGER,                 \n"
                           "  operation           TEXT,                    \n"
                           "  dynamic_node_rhs    INTEGER,                 \n"
                           "  hide_time           BOOLEAN    DEFAULT 1,    \n"
                           "  base_unit           INTEGER,                 \n"
                           "  document_dir        TEXT,                    \n"
                           "  value_decimal       INTEGER    DEFAULT 2,    \n"
                           "  ratio_decimal       INTEGER    DEFAULT 4     \n"
                           ");";

    QString section_rule_row = "INSERT INTO section_rule (static_node) VALUES (0);";

    QSqlQuery query {};
    if (db.transaction()) {
        // Execute each table creation query
        if (query.exec(finance) && query.exec(finance_path) && query.exec(finance_transaction) && query.exec(product) && query.exec(product_path)
            && query.exec(product_transaction) && query.exec(stakeholder) && query.exec(stakeholder_path) && query.exec(stakeholder_transaction)
            && query.exec(task) && query.exec(task_path) && query.exec(task_transaction) && query.exec(purchase) && query.exec(purchase_path)
            && query.exec(purchase_transaction) && query.exec(sales) && query.exec(sales_path) && query.exec(sales_transaction) && query.exec(section_rule)) {
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

QString MainwindowSql::NodeFinance(CString& table_name)
{
    return QString("CREATE TABLE IF NOT EXISTS %1 (           \n"
                   "  id INTEGER PRIMARY KEY AUTOINCREMENT,   \n"
                   "  name             TEXT,                  \n"
                   "  code             TEXT,                  \n"
                   "  description      TEXT,                  \n"
                   "  note             TEXT,                  \n"
                   "  node_rule        BOOLEAN    DEFAULT 0,  \n"
                   "  branch           BOOLEAN    DEFAULT 0,  \n"
                   "  unit             INTEGER,               \n"
                   "  initial_total    NUMERIC,               \n"
                   "  final_total      NUMERIC,               \n"
                   "  removed          BOOLEAN    DEFAULT 0   \n"
                   ");")
        .arg(table_name);
}

QString MainwindowSql::NodeTask(CString& table_name)
{
    return QString("CREATE TABLE IF NOT EXISTS %1 (           \n"
                   "  id INTEGER PRIMARY KEY AUTOINCREMENT,   \n"
                   "  name             TEXT,                  \n"
                   "  code             TEXT,                  \n"
                   "  description      TEXT,                  \n"
                   "  note             TEXT,                  \n"
                   "  node_rule        BOOLEAN    DEFAULT 0,  \n"
                   "  branch           BOOLEAN    DEFAULT 0,  \n"
                   "  unit             INTEGER,               \n"
                   "  initial_total    NUMERIC,               \n"
                   "  removed          BOOLEAN    DEFAULT 0   \n"
                   ");")
        .arg(table_name);
}

QString MainwindowSql::NodeStakeholder(CString& table_name)
{
    return QString("CREATE TABLE IF NOT EXISTS %1 (                           \n"
                   "  id INTEGER PRIMARY KEY AUTOINCREMENT,                   \n"
                   "  name              TEXT,                                 \n"
                   "  code              TEXT,                                 \n"
                   "  payment_period    INTEGER,                              \n"
                   "  employee          INTEGER,                              \n"
                   "  tax_rate          NUMERIC,                              \n"
                   "  deadline          TEXT,                                 \n"
                   "  description       TEXT,                                 \n"
                   "  note              TEXT,                                 \n"
                   "  term              BOOLEAN    DEFAULT 0,                 \n"
                   "  branch            BOOLEAN    DEFAULT 0,                 \n"
                   "  mark              INTEGER,                              \n"
                   "  removed           BOOLEAN    DEFAULT 0                  \n"
                   ");")
        .arg(table_name);
}

QString MainwindowSql::NodeProduct(CString& table_name)
{
    return QString("CREATE TABLE IF NOT EXISTS %1 (           \n"
                   "  id INTEGER PRIMARY KEY AUTOINCREMENT,   \n"
                   "  name             TEXT,                  \n"
                   "  code             TEXT,                  \n"
                   "  unit_price       NUMERIC,               \n"
                   "  commission       NUMERIC,               \n"
                   "  description      TEXT,                  \n"
                   "  note             TEXT,                  \n"
                   "  node_rule        BOOLEAN    DEFAULT 0,  \n"
                   "  branch           BOOLEAN    DEFAULT 0,  \n"
                   "  unit             INTEGER,               \n"
                   "  initial_total    NUMERIC,               \n"
                   "  removed          BOOLEAN    DEFAULT 0   \n"
                   ");")
        .arg(table_name);
}

QString MainwindowSql::NodeOrder(CString& table_name)
{
    return QString("CREATE TABLE IF NOT EXISTS %1 (                               \n"
                   "  id INTEGER PRIMARY KEY AUTOINCREMENT,                       \n"
                   "  name              TEXT,                                     \n"
                   "  first_property    INTEGER,                                  \n"
                   "  employee          INTEGER,                                  \n"
                   "  third_property    NUMERIC,                                  \n"
                   "  discount          NUMERIC,                                  \n"
                   "  refund            BOOLEAN    DEFAULT 0,                     \n"
                   "  stakeholder       INTEGER,                                  \n"
                   "  date_time         DATE,                                     \n"
                   "  description       TEXT,                                     \n"
                   "  posted            BOOLEAN    DEFAULT 0,                     \n"
                   "  branch            BOOLEAN    DEFAULT 0,                     \n"
                   "  mark              INTEGER,                                  \n"
                   "  initial_total     NUMERIC,                                  \n"
                   "  final_total       NUMERIC,                                  \n"
                   "  removed           BOOLEAN    DEFAULT 0                      \n"
                   ");")
        .arg(table_name);
}

QString MainwindowSql::Path(CString& table_name)
{
    return QString("CREATE TABLE IF NOT EXISTS %1 (                    \n"
                   "  ancestor      INTEGER  CHECK (ancestor   >= 1),  \n"
                   "  descendant    INTEGER  CHECK (descendant >= 1),  \n"
                   "  distance      INTEGER  CHECK (distance   >= 0)   \n"
                   ");")
        .arg(table_name);
}

QString MainwindowSql::TransactionFinance(CString& table_name)
{
    return QString("CREATE TABLE IF NOT EXISTS %1 (                                            \n"
                   "  id INTEGER PRIMARY KEY AUTOINCREMENT,                                    \n"
                   "  date_time      DATE,                                                     \n"
                   "  code           TEXT,                                                     \n"
                   "  lhs_node       INTEGER,                                                  \n"
                   "  lhs_ratio      NUMERIC    DEFAULT 1.0    CHECK (lhs_ratio  > 0),         \n"
                   "  lhs_debit      NUMERIC                   CHECK (lhs_debit  >=0),         \n"
                   "  lhs_credit     NUMERIC                   CHECK (lhs_credit >=0),         \n"
                   "  description    TEXT,                                                     \n"
                   "  transport      INTEGER                   CHECK(transport IN (0, 1, 2)),  \n"
                   "  location       TEXT,                                                     \n"
                   "  document       TEXT,                                                     \n"
                   "  state          BOOLEAN    DEFAULT 0,                                     \n"
                   "  rhs_credit     NUMERIC                   CHECK (rhs_credit >=0),         \n"
                   "  rhs_debit      NUMERIC                   CHECK (rhs_debit  >=0),         \n"
                   "  rhs_ratio      NUMERIC    DEFAULT 1.0    CHECK (rhs_ratio  > 0),         \n"
                   "  rhs_node       INTEGER,                                                  \n"
                   "  removed        BOOLEAN    DEFAULT 0                                      \n"
                   ");")
        .arg(table_name);
}

QString MainwindowSql::TransactionOrder(CString& table_name)
{
    return QString("CREATE TABLE IF NOT EXISTS %1 (                                            \n"
                   "  id INTEGER PRIMARY KEY AUTOINCREMENT,                                    \n"
                   "  date_time      DATE,                                                     \n"
                   "  code           TEXT,                                                     \n"
                   "  lhs_node       INTEGER,                                                  \n"
                   "  lhs_ratio      NUMERIC    DEFAULT 1.0    CHECK (lhs_ratio  > 0),         \n"
                   "  lhs_debit      NUMERIC                   CHECK (lhs_debit  >=0),         \n"
                   "  lhs_credit     NUMERIC                   CHECK (lhs_credit >=0),         \n"
                   "  description    TEXT,                                                     \n"
                   "  transport      INTEGER                   CHECK(transport IN (0, 1, 2)),  \n"
                   "  location       TEXT,                                                     \n"
                   "  document       TEXT,                                                     \n"
                   "  state          BOOLEAN    DEFAULT 0,                                     \n"
                   "  rhs_credit     NUMERIC                   CHECK (rhs_credit >=0),         \n"
                   "  rhs_debit      NUMERIC                   CHECK (rhs_debit  >=0),         \n"
                   "  rhs_ratio      NUMERIC    DEFAULT 1.0    CHECK (rhs_ratio  > 0),         \n"
                   "  rhs_node       INTEGER,                                                  \n"
                   "  removed        BOOLEAN    DEFAULT 0                                      \n"
                   ");")
        .arg(table_name);
}

QString MainwindowSql::TransactionStakeholder(CString& table_name)
{
    return QString("CREATE TABLE IF NOT EXISTS %1 (          \n"
                   "  id INTEGER PRIMARY KEY AUTOINCREMENT,  \n"
                   "  date_time      DATE,                   \n"
                   "  code           TEXT,                   \n"
                   "  lhs_node       INTEGER,                \n"
                   "  unit_price     NUMERIC,                \n"
                   "  commission     NUMERIC,                \n"
                   "  description    TEXT,                   \n"
                   "  document       TEXT,                   \n"
                   "  rhs_node       INTEGER,                \n"
                   "  removed        BOOLEAN    DEFAULT 0    \n"
                   ");")
        .arg(table_name);
}

QString MainwindowSql::TransactionTask(CString& table_name)
{
    return QString("CREATE TABLE IF NOT EXISTS %1 (                             \n"
                   "  id INTEGER PRIMARY KEY AUTOINCREMENT,                     \n"
                   "  date_time      DATE,                                      \n"
                   "  code           TEXT,                                      \n"
                   "  lhs_node       INTEGER,                                   \n"
                   "  lhs_ratio      NUMERIC,                                   \n"
                   "  lhs_debit      NUMERIC    CHECK (lhs_debit  >=0),         \n"
                   "  lhs_credit     NUMERIC    CHECK (lhs_credit >=0),         \n"
                   "  description    TEXT,                                      \n"
                   "  transport      INTEGER    CHECK(transport IN (0, 1, 2)),  \n"
                   "  location       TEXT,                                      \n"
                   "  document       TEXT,                                      \n"
                   "  state          BOOLEAN    DEFAULT 0,                      \n"
                   "  rhs_credit     NUMERIC    CHECK (rhs_credit >=0),         \n"
                   "  rhs_debit      NUMERIC    CHECK (rhs_debit  >=0),         \n"
                   "  rhs_ratio      NUMERIC,                                   \n"
                   "  rhs_node       INTEGER,                                   \n"
                   "  removed        BOOLEAN    DEFAULT 0                       \n"
                   ");")
        .arg(table_name);
}

QString MainwindowSql::TransactionProduct(CString& table_name)
{
    return QString("CREATE TABLE IF NOT EXISTS %1 (                             \n"
                   "  id INTEGER PRIMARY KEY AUTOINCREMENT,                     \n"
                   "  date_time      DATE,                                      \n"
                   "  code           TEXT,                                      \n"
                   "  lhs_node       INTEGER,                                   \n"
                   "  lhs_ratio      NUMERIC,                                   \n"
                   "  lhs_debit      NUMERIC    CHECK (lhs_debit  >=0),         \n"
                   "  lhs_credit     NUMERIC    CHECK (lhs_credit >=0),         \n"
                   "  description    TEXT,                                      \n"
                   "  transport      INTEGER    CHECK(transport IN (0, 1, 2)),  \n"
                   "  location       TEXT,                                      \n"
                   "  document       TEXT,                                      \n"
                   "  state          BOOLEAN    DEFAULT 0,                      \n"
                   "  rhs_credit     NUMERIC    CHECK (rhs_credit >=0),         \n"
                   "  rhs_debit      NUMERIC    CHECK (rhs_debit  >=0),         \n"
                   "  rhs_ratio      NUMERIC,                                   \n"
                   "  rhs_node       INTEGER,                                   \n"
                   "  removed        BOOLEAN    DEFAULT 0                       \n"
                   ");")
        .arg(table_name);
}
