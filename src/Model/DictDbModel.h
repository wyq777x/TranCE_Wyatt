#pragma once
#include <QString>
#include <SQLiteCpp/SQLiteCpp.h>
#include <memory>
#include <qtmetamacros.h>
class DictDbModel
{
public:
    DictDbModel () = delete;
    explicit DictDbModel (const QString &dbPath);

    DictDbModel (const DictDbModel &) = delete;
    DictDbModel operator= (const DictDbModel &) = delete;

    DictDbModel (DictDbModel &&) = default;
    DictDbModel &operator= (DictDbModel &&) = default;
    ~DictDbModel () = default;

private:
    std::unique_ptr<SQLite::Database> m_db;
};