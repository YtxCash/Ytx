#ifndef SQLCONNECTION_H
#define SQLCONNECTION_H

#include <QHash>
#include <QMutex>
#include <QSqlDatabase>

#include "component/enumclass.h"

class SqlConnection {
public:
    static SqlConnection& Instance();
    bool SetDatabaseName(const QString& file_path);
    QString DatabaseName() const;
    bool DatabaseEnable() const;
    QSqlDatabase* Allocate(Section section);

private:
    SqlConnection() = default;
    ~SqlConnection();

    SqlConnection(const SqlConnection&) = delete;
    SqlConnection& operator=(const SqlConnection&) = delete;
    SqlConnection(SqlConnection&&) = delete;
    SqlConnection& operator=(SqlConnection&&) = delete;

private:
    QMutex mutex_;
    QHash<Section, QSqlDatabase> hash_;

    QString file_path_ {};
    bool database_enable_ { false };
};

#endif // SQLCONNECTION_H
