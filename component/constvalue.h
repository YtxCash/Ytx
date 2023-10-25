#ifndef CONSTVALUE_H
#define CONSTVALUE_H

// Constants for values
constexpr double DMAX = 1'000'000'000.00;
constexpr double DMIN = -1'000'000'000.00;
constexpr int HUNDRED = 100;
constexpr int IMAX = 1'000'000'000;
constexpr int IMIN = -1'000'000'000;
constexpr int ROW_HEIGHT = 24;

// Constants for datetime
constexpr char DATE_D[] = "d";
constexpr char DATE_FORMAT[] = "date_format";
constexpr char DATE_FST[] = "yyyy-MM-dd";
constexpr char DATE_TIME[] = "date_time";
constexpr char DATE_TIME_FST[] = "yyyy-MM-dd hh:mm:ss";

// Constants for separators
constexpr char COLON[] = ":";
constexpr char DASH[] = "-";
constexpr char SEPARATOR[] = "separator";
constexpr char SLASH[] = "/";

// Constants for operators
constexpr char MINUS[] = "-";
constexpr char PLUS[] = "+";

// Constants for table column check state, kState
constexpr char CHECK[] = "check";

// Constants for files' suffix
constexpr char SFX_INI[] = ".ini";
constexpr char SFX_LOCK[] = ".lock";
constexpr char SFX_QM[] = ".qm";
constexpr char SFX_QSS[] = ".qss";
constexpr char SFX_YTX[] = ".ytx";
constexpr char SFX_PERCENT[] = "%";

// Constants for app's language
constexpr char EN_US[] = "en_US";
constexpr char LANGUAGE[] = "language";
constexpr char ZH_CN[] = "zh_CN";

// Constants for tree and table's column
constexpr char BRANCH[] = "branch";
constexpr char CODE[] = "code";
constexpr char COMMISSION[] = "commission";
constexpr char DEADLINE[] = "deadline";
constexpr char DECIMAL[] = "decimal";
constexpr char DESCRIPTION[] = "description";
constexpr char DISCOUNT[] = "discount";
constexpr char DOCUMENT[] = "document";
constexpr char EMPLOYEE[] = "employee";
constexpr char FINAL_TOTAL[] = "final_total";
constexpr char FIRST_PROPERTY[] = "first_property";
constexpr char INITIAL_TOTAL[] = "initial_total";
constexpr char LOCATION[] = "location";
constexpr char NAME[] = "name";
constexpr char NODE_ID[] = "node/id";
constexpr char NODE_RULE[] = "node_rule";
constexpr char NOTE[] = "note";
constexpr char PAYMENT_PERIOD[] = "payment_period";
constexpr char REFUND[] = "refund";
constexpr char STATE[] = "state";
constexpr char TAX_RATE[] = "tax_rate";
constexpr char TERM[] = "term";
constexpr char THIRD_PROPERTY[] = "third_property";
constexpr char TRANSPORT[] = "transport";
constexpr char UNIT[] = "unit";
constexpr char UNIT_PRICE[] = "unit_price";

// Constants for app's state
constexpr char HEADER_STATE[] = "header_state";
constexpr char INTERFACE[] = "interface";
constexpr char MAINWINDOW_GEOMETRY[] = "mainwindow_geometry";
constexpr char MAINWINDOW_STATE[] = "mainwindow_state";
constexpr char SOLARIZED_DARK[] = "Solarized Dark";
constexpr char SPLITTER_STATE[] = "splitter_state";
constexpr char THEME[] = "theme";
constexpr char VIEW[] = "view";
constexpr char WINDOW[] = "window";

// Constants for others
constexpr char QSQLITE[] = "QSQLITE";
constexpr char RECENT_FILE[] = "recent/file";
constexpr char SEMICOLON[] = ";";
constexpr char START_SECTION[] = "start/section";
constexpr char YTX[] = "ytx";

// Constants for sections
constexpr char Finance[] = "Finance";
constexpr char FINANCE[] = "finance";
constexpr char FINANCE_PATH[] = "finance_path";
constexpr char FINANCE_TRANSACTION[] = "finance_transaction";
constexpr char Product[] = "Product";
constexpr char PRODUCT[] = "product";
constexpr char PRODUCT_PATH[] = "product_path";
constexpr char PRODUCT_TRANSACTION[] = "product_transaction";
constexpr char Purchase[] = "Purchase";
constexpr char PURCHASE[] = "purchase";
constexpr char PURCHASE_PATH[] = "purchase_path";
constexpr char PURCHASE_TRANSACTION[] = "purchase_transaction";
constexpr char Sales[] = "Sales";
constexpr char SALES[] = "sales";
constexpr char SALES_PATH[] = "sales_path";
constexpr char SALES_TRANSACTION[] = "sales_transaction";
constexpr char Stakeholder[] = "Stakeholder";
constexpr char STAKEHOLDER[] = "stakeholder";
constexpr char STAKEHOLDER_PATH[] = "stakeholder_path";
constexpr char STAKEHOLDER_TRANSACTION[] = "stakeholder_transaction";
constexpr char Task[] = "Task";
constexpr char TASK[] = "task";
constexpr char TASK_PATH[] = "task_path";
constexpr char TASK_TRANSACTION[] = "task_transaction";

#endif // CONSTVALUE_H
