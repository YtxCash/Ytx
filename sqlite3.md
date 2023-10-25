# sqlite

```sql

CREATE TABLE IF NOT EXISTS finance
(
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  name             TEXT,
  code             TEXT,
  description      TEXT,
  note             TEXT,
  node_rule        BOOLEAN    DEFAULT 0,        -- DDCI = 1, DICD = 0
  branch           BOOLEAN    DEFAULT 0,
  unit             INTEGER,
  base_total       NUMERIC,
  foreign_total    NUMERIC,
  removed          BOOLEAN    DEFAULT 0
);

CREATE TABLE IF NOT EXISTS finance_path
(
  ancestor      INTEGER    CHECK (ancestor   >= 1),
  descendant    INTEGER    CHECK (descendant >= 1),
  distance      INTEGER    CHECK (distance   >= 0)

  -- PRIMARY KEY (descendant, distance, ancestor)
);

-- CREATE INDEX finance_path_idx ON finance_path (ancestor, distance);

CREATE TABLE IF NOT EXISTS finance_transaction
(
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  transport      INTEGER                      CHECK(transport IN (-1, 0, 1)),
  location       TEXT,
  date_time      DATE,
  code           TEXT,
  lhs_node       INTEGER,
  lhs_ratio      NUMERIC       DEFAULT 1.0    CHECK (lhs_ratio  > 0),
  lhs_debit      NUMERIC                      CHECK (lhs_debit  >=0),
  lhs_credit     NUMERIC                      CHECK (lhs_credit >=0),
  description    TEXT,
  rhs_node       INTEGER,
  rhs_ratio      NUMERIC       DEFAULT 1.0    CHECK (rhs_ratio  > 0),
  rhs_debit      NUMERIC                      CHECK (rhs_debit  >=0),
  rhs_credit     NUMERIC                      CHECK (rhs_credit >=0),
  state          BOOLEAN       DEFAULT 0,
  document       TEXT,
  removed        BOOLEAN       DEFAULT 0
);

CREATE TABLE IF NOT EXISTS product
(
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  name             TEXT,
  code             TEXT,
  unit_price       NUMERIC,
  description      TEXT,
  note             TEXT,
  node_rule        BOOLEAN    DEFAULT 0,        -- DDCI = 1, DICD = 0
  branch           BOOLEAN    DEFAULT 0,
  unit             INTEGER,
  foreign_total    NUMERIC,
  removed          BOOLEAN    DEFAULT 0
);

CREATE TABLE IF NOT EXISTS product_path
(
  ancestor      INTEGER    CHECK (ancestor   >= 1),
  descendant    INTEGER    CHECK (descendant >= 1),
  distance      INTEGER    CHECK (distance   >= 0)

  -- PRIMARY KEY (descendant, distance, ancestor)
);

-- CREATE INDEX product_path_idx ON product_path (ancestor, distance);

CREATE TABLE IF NOT EXISTS product_transaction
(
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  transport      INTEGER                      CHECK(transport IN (-1, 0, 1)),
  location       TEXT,
  date_time      DATE,
  code           TEXT,
  lhs_node       INTEGER,
  lhs_ratio      NUMERIC       DEFAULT 1.0    CHECK (lhs_ratio  > 0),
  lhs_debit      NUMERIC                      CHECK (lhs_debit  >=0),
  lhs_credit     NUMERIC                      CHECK (lhs_credit >=0),
  description    TEXT,
  rhs_node       INTEGER,
  rhs_ratio      NUMERIC       DEFAULT 1.0    CHECK (rhs_ratio  > 0),
  rhs_debit      NUMERIC                      CHECK (rhs_debit  >=0),
  rhs_credit     NUMERIC                      CHECK (rhs_credit >=0),
  state          BOOLEAN       DEFAULT 0,
  document       TEXT,
  removed        BOOLEAN       DEFAULT 0
);

CREATE TABLE IF NOT EXISTS network
(
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  name            TEXT,
  code            TEXT,
  interval        INTEGER, 
  tax_rate        NUMERIC,
  date_time       DATE,
  description     TEXT,
  note            TEXT,
  branch          BOOLEAN    DEFAULT 0,
  unit            INTEGER,
  removed         BOOLEAN    DEFAULT 0
);

CREATE TABLE IF NOT EXISTS network_path
(
  ancestor      INTEGER    CHECK (ancestor   >= 1),
  descendant    INTEGER    CHECK (descendant >= 1),
  distance      INTEGER    CHECK (distance   >= 0)

  -- PRIMARY KEY (descendant, distance, ancestor)
);

-- CREATE INDEX network_path_idx ON network_path (ancestor, distance);

CREATE TABLE IF NOT EXISTS network_transaction
(
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  transport      INTEGER                      CHECK(transport IN (-1, 0, 1)),
  location       TEXT,
  date_time      DATE,
  code           TEXT,
  lhs_node       INTEGER,
  lhs_ratio      NUMERIC       DEFAULT 1.0    CHECK (lhs_ratio  > 0),
  lhs_debit      NUMERIC                      CHECK (lhs_debit  >=0),
  lhs_credit     NUMERIC                      CHECK (lhs_credit >=0),
  description    TEXT,
  rhs_node       INTEGER,
  rhs_ratio      NUMERIC       DEFAULT 1.0    CHECK (rhs_ratio  > 0),
  rhs_debit      NUMERIC                      CHECK (rhs_debit  >=0),
  rhs_credit     NUMERIC                      CHECK (rhs_credit >=0),
  state          BOOLEAN       DEFAULT 0,
  document       TEXT,
  removed        BOOLEAN       DEFAULT 0
);

CREATE TABLE IF NOT EXISTS task
(
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  name             TEXT,
  code             TEXT,
  description      TEXT,
  note             TEXT,
  node_rule        BOOLEAN    DEFAULT 0,        -- DDCI = 1, DICD = 0
  branch           BOOLEAN    DEFAULT 0,
  unit             INTEGER,
  foreign_total    NUMERIC,
  removed          BOOLEAN    DEFAULT 0
);

CREATE TABLE IF NOT EXISTS task_path
(
  ancestor      INTEGER    CHECK (ancestor   >= 1),
  descendant    INTEGER    CHECK (descendant >= 1),
  distance      INTEGER    CHECK (distance   >= 0)

  -- PRIMARY KEY (descendant, distance, ancestor)
);

-- CREATE INDEX task_path_idx ON task_path (ancestor, distance);

CREATE TABLE IF NOT EXISTS task_transaction
(
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  transport      INTEGER                      CHECK(transport IN (-1, 0, 1)),
  location       TEXT,
  date_time      DATE,
  code           TEXT,
  lhs_node       INTEGER,
  lhs_ratio      NUMERIC       DEFAULT 1.0    CHECK (lhs_ratio  > 0),
  lhs_debit      NUMERIC                      CHECK (lhs_debit  >=0),
  lhs_credit     NUMERIC                      CHECK (lhs_credit >=0),
  description    TEXT,
  rhs_node       INTEGER,
  rhs_ratio      NUMERIC       DEFAULT 1.0    CHECK (rhs_ratio  > 0),
  rhs_debit      NUMERIC                      CHECK (rhs_debit  >=0),
  rhs_credit     NUMERIC                      CHECK (rhs_credit >=0),
  state          BOOLEAN       DEFAULT 0,
  document       TEXT,
  removed        BOOLEAN       DEFAULT 0
);

CREATE TABLE IF NOT EXISTS section_rule
(
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  -- 1-finance,  2-sales, 3-task, 4-network, 5-product, 6-purchase
  static_label        TEXT,
  static_node         INTEGER,
  dynamic_label       TEXT,
  dynamic_node_lhs    INTEGER,
  operation           TEXT,
  dynamic_node_rhs    INTEGER,
  hide_time           BOOLEAN    DEFAULT TRUE,
  base_unit           INTEGER,
  document_dir        TEXT,
  value_decimal       INTEGER    DEFAULT 2,
  ratio_decimal       INTEGER    DEFAULT 4
);

INSERT INTO section_rule (static_node) VALUES (0);
INSERT INTO section_rule (static_node) VALUES (0);
INSERT INTO section_rule (static_node) VALUES (0);
INSERT INTO section_rule (static_node) VALUES (0);
INSERT INTO section_rule (static_node) VALUES (0);
INSERT INTO section_rule (static_node) VALUES (0);

```
