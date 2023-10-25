#include "sqlconnection.h"

#include <QDebug>

#include "component/constvalue.h"

SqlConnection& SqlConnection::Instance()
{
    static SqlConnection instance;
    return instance;
}

bool SqlConnection::SetDatabaseName(const QString& file_path)
{
    QMutexLocker locker(&mutex_);

    if (file_path.endsWith(SFX_YTX, Qt::CaseInsensitive)) {
        auto db { QSqlDatabase::addDatabase(QSQLITE, QString::number(0)) };
        db.setDatabaseName(file_path);

        if (db.open()) {
            hash_.insert(Section(0), db);
            file_path_ = file_path;
            return database_enable_ = true;
        }
    }

    qDebug() << "Fail to oepn file " << file_path;
    return false;
}

QString SqlConnection::DatabaseName() const { return file_path_; }

QSqlDatabase* SqlConnection::Allocate(Section section)
{
    QMutexLocker locker(&mutex_);

    if (!database_enable_) {
        qDebug() << "Set database file first ";
        return nullptr;
    }

    if (!hash_.contains(section)) {
        auto db { QSqlDatabase::addDatabase(QSQLITE, QString::number(static_cast<int>(section))) };
        db.setDatabaseName(file_path_);

        if (db.open())
            hash_.insert(section, db);
        else {
            qDebug() << "Fail to create " << file_path_ << " connection " << static_cast<int>(section);
            return nullptr;
        }
    }

    return &hash_[section];
}

bool SqlConnection::DatabaseEnable() const { return database_enable_; }

SqlConnection::~SqlConnection()
{
    for (auto& db : hash_)
        if (db.isOpen())
            db.close();

    hash_.clear();
}
