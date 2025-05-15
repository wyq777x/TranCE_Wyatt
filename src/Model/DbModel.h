#pragma once
#include "SQLiteCpp/Database.h"
#include "Utility/UserAuthResult.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QString>
#include <SQLiteCpp/SQLiteCpp.h>
#include <Utility/WordEntry.h>
#include <memory>
#include <optional>
#include <qtmetamacros.h>
#include <vector>
class DbModel
{
public:
    DbModel () = delete;
    explicit DbModel (const QString &dbPath);

    DbModel (const DbModel &) = delete;
    DbModel operator= (const DbModel &) = delete;

    DbModel (DbModel &&) = default;
    DbModel &operator= (DbModel &&) = default;
    ~DbModel () = default;

    void initDBs ()
    {
        QString dbDir =
            QCoreApplication::applicationDirPath () + "/Utility/DBs";

        QDir dir (dbDir);
        if (!dir.exists ())
        {
            dir.mkpath (".");
        }

        QString userDbPath = dir.filePath ("user.db");
        if (!QFile::exists (userDbPath))
        {
            user_db = std::make_unique<SQLite::Database> (
                userDbPath.toStdString (),
                SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
            initUserTable ();
        }
        else if (!isUserDbOpen ())
        {
            user_db = std::make_unique<SQLite::Database> (
                userDbPath.toStdString (), SQLite::OPEN_READWRITE);
        }

        QString dictDbPath = dir.filePath ("dict.db");
        if (!QFile::exists (dictDbPath))
        {
            dict_db = std::make_unique<SQLite::Database> (
                dictDbPath.toStdString (),
                SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
            initDictTable ();
        }
        else if (!isDictDbOpen ())
        {
            dict_db = std::make_unique<SQLite::Database> (
                userDbPath.toStdString (), SQLite::OPEN_READWRITE);
        }
    }

    // User

    void registerUser (const QString &username, const QString &passwordHash);
    bool userExists (const QString &username) const;
    bool validateUser (const QString &username,
                       const QString &passwordHash) const;

    UserAuthResult verifyUser (const QString &username,
                               const QString &passwordHash) const;
    bool updateUserPassword (const QString &username,
                             const QString &oldPasswordHash,
                             const QString &newPasswordHash);

    std::optional<QString> getUserId (const QString &username) const;
    std::optional<QString> getUserName (const QString &userId) const;

    // Database connection and management
    void openDb (const QString &dbPath);
    void closeDb ();
    bool isUserDbOpen () const;
    bool isDictDbOpen () const;

    void initUserTable ()
    {

        try
        {
            if (user_db != nullptr)
            {
                user_db->exec ("CREATE TABLE IF NOT EXISTS users("
                               "user_id TEXT PRIMARY KEY,"
                               "username TEXT NOT NULL UNIQUE,"
                               "password_hash TEXT NOT NULL,"
                               "email TEXT NOT NULL UNIQUE);");
            }
        }
        catch (const SQLite::Exception &e)
        {
            logErr ("Error creating user table", e);
        }
    }

    void initDictTable ()
    {
        try
        {
            if (dict_db != nullptr)
            {
                dict_db->exec ("CREATE TABLE IF NOT EXISTS words();");

                // to be further designed
            }
        }
        catch (const SQLite::Exception &e)
        {
            logErr ("Error creating dictionary table", e);
        }
    }

    std::optional<WordEntry> lookupWord (const QString &word,
                                         const QString &lang);
    std::vector<WordEntry> searchWords (const QString &pattern,
                                        const QString &lang, int limit = 10);

    bool addToUserVocabulary (const QString &userId, const QString &word,
                              const QString &lang);
    bool removeFromUserVocabulary (const QString &userId, const QString &word);

    // -1= never learned,0=learning, 1=mastered
    bool updateWordStatus (const QString &userId, const QString &word,
                           int status);

    std::vector<WordEntry> getUserVocabulary (const QString &userId,
                                              int status = -1); // -1 means all

    QString getLastError () const;

private:
    std::unique_ptr<SQLite::Database> user_db;
    std::unique_ptr<SQLite::Database> dict_db;
    QString m_lastError;
    void logErr (const std::string &errMsg, const SQLite::Exception &e) const;
};