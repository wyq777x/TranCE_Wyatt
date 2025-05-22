#pragma once
#include "SQLiteCpp/Database.h"
#include "Utility/Result.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QString>
#include <SQLiteCpp/SQLiteCpp.h>
#include <Utility/WordEntry.h>
#include <memory>
#include <optional>
#include <qcontainerfwd.h>
#include <qtmetamacros.h>
#include <vector>
class DbModel
{
public:
    DbModel () = delete;
    explicit DbModel (const QString &dbPath)
        : m_dbDir (dbPath), user_db (nullptr), dict_db (nullptr)
    {
        initDBs ();
    }

    DbModel (const DbModel &) = delete;
    DbModel operator= (const DbModel &) = delete;

    DbModel (DbModel &&) = default;
    DbModel &operator= (DbModel &&) = default;
    ~DbModel () = default;

    void initDBs ()
    {
        //  QString dbDir =
        //  QCoreApplication::applicationDirPath () + "/Utility/DBs";

        QDir dir (m_dbDir);
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

    bool userExists (const QString &username) const;
    RegisterUserResult registerUser (const QString &username,
                                     const QString &passwordHash);

    UserAuthResult verifyUser (const QString &username,
                               const QString &passwordHash) const;
    bool updateUserPassword (const QString &username,
                             const QString &oldPasswordHash,
                             const QString &newPasswordHash);

    std::optional<QString> getUserId (const QString &username) const;
    std::optional<QString> getUserName (const QString &userId) const;

    // Database connection and management

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
                               "email TEXT UNIQUE);");
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

    bool addToUserVocabulary (const QString &userId, const QString &word);
    bool removeFromUserVocabulary (const QString &userId, const QString &word);

    // status: -1= never learned,0=learning, 1=mastered
    bool updateWordStatus (const QString &userId, const QString &word,
                           int status);

    std::vector<WordEntry> getUserVocabulary (const QString &userId,
                                              int status = -1); // -1 means all

    std::string getLastError () const { return m_lastError; }

private:
    QString m_dbDir;
    std::unique_ptr<SQLite::Database> user_db;
    std::unique_ptr<SQLite::Database> dict_db;
    mutable std::string m_lastError;

    template <typename ExceptionT>
    void logErr (const std::string &errMsg, const ExceptionT &e) const
    {
        qCritical () << "[DbModel]" << QString::fromStdString (errMsg)
                     << "Exception:" << e.what ();
        m_lastError = errMsg + " Exception:" + std::string (e.what ());
    }
};