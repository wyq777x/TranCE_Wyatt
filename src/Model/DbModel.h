#pragma once
#include "SQLiteCpp/Database.h"
#include "Utility/Result.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QString>
#include <QUuid>
#include <SQLiteCpp/SQLiteCpp.h>
#include <Utility/AsyncTask.h>
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

    DbModel (const DbModel &) = delete;
    DbModel operator= (const DbModel &) = delete;

    DbModel (DbModel &&) = default;
    DbModel &operator= (DbModel &&) = default;
    ~DbModel () = default;

    static DbModel &getInstance ()
    {
        static DbModel instance (QCoreApplication::applicationDirPath () +
                                 "/Utility/DBs");
        return instance;
    }

    void initDBs ()
    {
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
        bool isNewDictDb = !QFile::exists (dictDbPath);
        if (isNewDictDb)
        {
            dict_db = std::make_unique<SQLite::Database> (
                dictDbPath.toStdString (),
                SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
            initDictTable ();
            m_needsDictImport = true; // mark as needing import
        }
        else if (!isDictDbOpen ())
        {
            dict_db = std::make_unique<SQLite::Database> (
                dictDbPath.toStdString (), SQLite::OPEN_READWRITE);
        }
    }

    // Asynchronous init method
    AsyncTask<void> initializeAsync (const QString &dictPath = "");
    // Database connection and management

    bool isUserDbOpen () const;
    bool isDictDbOpen () const;

    // User

    bool userExists (const QString &username) const;

    static RegisterUserResult registerUser (const QString &username,
                                            const QString &passwordHash);

    UserAuthResult verifyUser (const QString &username,
                               const QString &passwordHash) const;

    void deleteUser (const QString &username);

    ChangeResult updateUserPassword (const QString &username,
                                     const QString &oldPasswordHash,
                                     const QString &newPasswordHash);

    ChangeResult changeUsername (const QString &oldUsername,
                                 const QString &newUsername);
    ChangeResult changeEmail (const QString &username, const QString &oldEmail,
                              const QString &newEmail);

    std::optional<QString> getUserId (const QString &username) const;
    std::optional<QString> getUserName (const QString &userId) const;

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
        catch (const std::exception &e)
        {
            logErr ("Unknown error creating user table", e);
        }
        catch (...)
        {
            logErr ("Unknown error creating user table",
                    std::runtime_error ("Unknown exception"));
        }
    }

    void initDictTable ()
    {
        try
        {

            dict_db->exec ("PRAGMA journal_mode = WAL");
            dict_db->exec ("PRAGMA synchronous = NORMAL");
            dict_db->exec ("PRAGMA cache_size = 20000");
            dict_db->exec ("PRAGMA temp_store = MEMORY");
            dict_db->exec ("PRAGMA locking_mode = EXCLUSIVE");
            dict_db->exec ("PRAGMA page_size = 8192");

            if (dict_db != nullptr)
            {

                // Create "users"

                dict_db->exec ("CREATE TABLE IF NOT EXISTS users("
                               "user_id TEXT PRIMARY KEY,"
                               "username TEXT NOT NULL UNIQUE);");

                // Main dictionary table "words"
                dict_db->exec (
                    "CREATE TABLE IF NOT EXISTS words("
                    "word_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "word TEXT NOT NULL UNIQUE,"
                    "part_of_speech TEXT,"
                    "pronunciation TEXT,"
                    "frequency INTEGER DEFAULT 0,"
                    "notes TEXT,"
                    "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                    "updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP);");

                // Translations relationship table "translations"

                dict_db->exec (
                    "CREATE TABLE IF NOT EXISTS word_translations("
                    "translation_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "source_word TEXT NOT NULL,"     // for example : "apple"
                    "target_word TEXT NOT NULL,"     // for example : "苹果"
                    "source_language TEXT NOT NULL," // for example : "en"
                    "target_language TEXT NOT NULL," // for example : "zh"
                    "confidence_score REAL DEFAULT 1.0 CHECK (confidence_score "
                    ">= 0.0 AND confidence_score <= 1.0),"
                    "UNIQUE(source_word,target_word,source_language,target_"
                    "language));");

                // Example sentences table "examples",only for English words to
                // Chinese sentences

                dict_db->exec ("CREATE TABLE IF NOT EXISTS examples("
                               "example_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                               "word TEXT NOT NULL,"
                               "example_sentence TEXT NOT NULL,"
                               "translation TEXT,"
                               "FOREIGN KEY(word) REFERENCES words(word)"
                               "ON DELETE CASCADE ON UPDATE CASCADE);");

                // User learning progress table "user_progress"

                dict_db->exec (
                    "CREATE TABLE IF NOT EXISTS user_progress("
                    "user_id TEXT NOT NULL," // write from user.db every time
                                             // when user logs in (MUST)
                    "source_word TEXT NOT NULL,"
                    "target_word TEXT NOT NULL,"
                    "source_language TEXT NOT NULL,"
                    "target_language TEXT NOT NULL,"
                    "correct_count INTEGER DEFAULT 0,"
                    "incorrect_count INTEGER DEFAULT 0,"
                    "last_reviewed TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                    "mastery_level INTEGER DEFAULT 0,"
                    "study_streak INTEGER DEFAULT 0,"
                    "PRIMARY KEY (user_id, source_word, target_word, "
                    "source_language, target_language),"
                    "FOREIGN KEY(source_word) REFERENCES words(word) ON DELETE "
                    "CASCADE,"
                    "FOREIGN KEY(target_word) REFERENCES words(word) ON DELETE "
                    "CASCADE,"
                    "FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE "
                    "CASCADE);");

                // Index for boosting performance
                dict_db->exec ("CREATE INDEX IF NOT EXISTS idx_words_word ON "
                               "words(word);");
                dict_db->exec (
                    "CREATE INDEX IF NOT EXISTS idx_translations_source ON "
                    "word_translations(source_word, source_language);");
                dict_db->exec (
                    "CREATE INDEX IF NOT EXISTS idx_translations_target ON "
                    "word_translations(target_word, target_language);");
                dict_db->exec ("CREATE INDEX IF NOT EXISTS idx_examples_word "
                               "ON examples(word);");
            }
        }
        catch (const SQLite::Exception &e)
        {
            logErr ("Error creating dictionary table", e);
        }
        catch (const std::exception &e)
        {
            logErr ("Unknown error creating dictionary table", e);
        }
        catch (...)
        {
            logErr ("Unknown error creating dictionary table",
                    std::runtime_error ("Unknown exception"));
        }
    }

    // import single WordEntry
    AsyncTask<void> importWordEntry (const WordEntry &wordEntry);

    // import multiple WordEntry
    AsyncTask<void>
    importWordEntriesAsync (const std::vector<WordEntry> &wordEntries);

    // import multiple WordEntry from file

    AsyncTask<void> importFromFileAsync (
        const QString &filePath,
        std::function<void (int, int)> progressCallback = nullptr);

    // Synchronous import method (backup for when coroutines don't work)
    void importFromFileSync (
        const QString &filePath,
        std::function<void (int, int)> progressCallback = nullptr);

    AsyncTask<void> insertWordBatch (const std::vector<WordEntry> &batch);

    // Synchronous batch insert helper
    void insertWordBatchSync (const std::vector<WordEntry> &batch);

    std::optional<WordEntry> lookupWord (const QString &word,
                                         const QString &lang);
    std::vector<WordEntry> searchWords (const QString &pattern,
                                        const QString &lang, int limit = 10);

    void addToUserVocabulary (const QString &userId, const QString &word);
    void removeFromUserVocabulary (const QString &userId, const QString &word);

    // status: -1= never learned,0=learning, 1=mastered
    void updateWordStatus (const QString &userId, const QString &word,
                           int status);

    std::vector<WordEntry> getUserVocabulary (const QString &userId,
                                              int status = -1); // -1 means all

    std::string getLastError () const { return m_lastError; }

private:
    explicit DbModel (const QString &dbPath)
        : m_dbDir (dbPath), user_db (nullptr), dict_db (nullptr)
    {
        initDBs ();
    }

    QString m_dbDir;
    std::unique_ptr<SQLite::Database> user_db;
    std::unique_ptr<SQLite::Database> dict_db;
    mutable std::string m_lastError;
    bool m_needsDictImport = false;

    template <typename ExceptionT>
    void logErr (const std::string &errMsg, const ExceptionT &e) const
    {
        qCritical () << "[DbModel]" << QString::fromStdString (errMsg)
                     << "Exception:" << e.what ();
        m_lastError = errMsg + " Exception:" + std::string (e.what ());
    }

    WordEntry parseCSVLineToWordEntry (const QString &csvLine);

    QStringList parseCSVLine (const QString &line);
};
