#pragma once
#include "SQLiteCpp/Database.h"
#include "Utility/AsyncTask.h"
#include "Utility/Constants.h"
#include "Utility/Result.h"
#include "Utility/WordEntry.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QString>
#include <QUuid>
#include <SQLiteCpp/SQLiteCpp.h>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

class DbModel
{
public:
    DbModel (const DbModel &) = delete;
    DbModel operator= (const DbModel &) = delete;

    DbModel (DbModel &&) = default;
    DbModel &operator= (DbModel &&) = default;
    ~DbModel () = default;

    static DbModel &getInstance ()
    {
        static DbModel instance;
        return instance;
    }

    static QString getDbDir ()
    {
        return QCoreApplication::applicationDirPath () +
               Constants::Paths::DB_DIR;
    }

    static QString getAvatarDir ()
    {
        return QCoreApplication::applicationDirPath () +
               Constants::Paths::AVATAR_DIR;
    }

    void initDBs ()
    {
        QDir dir (getDbDir ());
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

    bool userIdExists (const QString &userId) const;

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

    ChangeResult changeAvatar (const QString &username,
                               const QString &avatarPath);

    ChangeResult updateReciteProgress (int current, int total,
                                       const QString &userId);

    std::optional<QString> getUserId (const QString &username) const;
    std::optional<QString> getUserName (const QString &userId) const;
    QString getUserEmail (const QString &username) const;
    QString getUserAvatarPath (const QString &username) const;

    void initUserTable ()
    {

        try
        {
            user_db->exec ("PRAGMA journal_mode = WAL");
            user_db->exec ("PRAGMA synchronous = NORMAL");
            user_db->exec ("PRAGMA cache_size = 10000");
            user_db->exec ("PRAGMA temp_store = MEMORY");
            user_db->exec ("PRAGMA locking_mode = NORMAL");
            user_db->exec ("PRAGMA page_size = 4096");
            user_db->exec ("PRAGMA foreign_keys = ON");

            if (user_db != nullptr)
            {
                user_db->exec (
                    "CREATE TABLE IF NOT EXISTS users("
                    "user_id TEXT PRIMARY KEY,"
                    "username TEXT NOT NULL UNIQUE,"
                    "password_hash TEXT NOT NULL,"
                    "email TEXT UNIQUE,"
                    "avatar_path TEXT DEFAULT ':/image/DefaultUser.png');");

                user_db->exec (
                    "CREATE TABLE IF NOT EXISTS user_search_history("
                    "search_history_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "user_id TEXT NOT NULL,"
                    "search_word TEXT NOT NULL,"
                    "FOREIGN KEY(user_id) REFERENCES users(user_id) "
                    "ON DELETE CASCADE "
                    "ON UPDATE CASCADE );");

                user_db->exec (
                    "CREATE TABLE IF NOT EXISTS user_recite_history("
                    " recite_history_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    " user_id TEXT NOT NULL,"
                    " recite_word TEXT NOT NULL,"
                    " FOREIGN KEY(user_id) REFERENCES users(user_id) "
                    "ON DELETE CASCADE ON UPDATE CASCADE );");

                user_db->exec ("CREATE TABLE IF NOT EXISTS user_progress("
                               "user_id TEXT PRIMARY KEY,"
                               "total_words INTEGER DEFAULT 15,"
                               "current_progress INTEGER DEFAULT 0 CHECK "
                               "(current_progress >= 0 AND current_progress <= "
                               "total_words), "
                               "FOREIGN KEY(user_id) REFERENCES users(user_id) "
                               "ON DELETE CASCADE ON UPDATE CASCADE);");

                user_db->exec (
                    "CREATE TABLE IF NOT EXISTS user_favorites("
                    "favorites_word_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "user_id TEXT NOT NULL,"
                    "word TEXT NOT NULL,"
                    "FOREIGN KEY(user_id) REFERENCES users(user_id) "
                    "ON DELETE CASCADE ON UPDATE CASCADE );");

                user_db->exec ("CREATE TABLE IF NOT EXISTS user_vocabulary("
                               "user_id TEXT NOT NULL,"
                               "word TEXT NOT NULL,"
                               "status INTEGER DEFAULT -1 CHECK (status IN ( "
                               "0, 1))," // status:
                                         // 0= learning,
                                         // 1= mastered
                               "FOREIGN KEY(user_id) REFERENCES users(user_id) "
                               "ON DELETE CASCADE ON UPDATE CASCADE,"
                               "PRIMARY KEY (user_id, word));");

                user_db->exec (
                    "CREATE INDEX IF NOT EXISTS idx_user_favorites ON "
                    "user_favorites(favorites_word_id, user_id);");

                user_db->exec (
                    "CREATE INDEX IF NOT EXISTS idx_user_search_history "
                    "ON user_search_history(search_history_id, user_id);");

                user_db->exec (
                    "CREATE INDEX IF NOT EXISTS idx_user_recite_history "
                    "ON user_recite_history(recite_history_id, user_id);");
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
            dict_db->exec ("PRAGMA locking_mode = NORMAL");
            dict_db->exec ("PRAGMA page_size = 8192");
            dict_db->exec ("PRAGMA foreign_keys = ON");

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
                                         const QString &srcLang);
    std::vector<WordEntry> searchWords (const QString &pattern,
                                        const QString &srcLang, int limit = 10);

    void addToUserFavorites (const QString &userId, const QString &word);
    void removeFromUserFavorites (const QString &userId, const QString &word);
    bool isWordFavorited (const QString &userId, const QString &word) const;

    bool existsInMastered (const QString &userId, const QString &word) const;

    // status: 0=learning, 1=mastered
    ChangeResult updateWordStatus (const QString &userId, const QString &word,
                                   int status);

    std::vector<QString> getUserVocabulary (const QString &userId,
                                            int status = -1); // -1 means all

    std::vector<QString> getUserFavorites (const QString &userId);

    std::string getLastError () const { return m_lastError; }

    std::optional<WordEntry> getRandomWord ();

    void addToSearchHistory (const QString &userId, const QString &word);
    void removeFromSearchHistory (const QString &userId, const QString &word);

    void addToReciteHistory (const QString &userId, const QString &word);
    void removeFromReciteHistory (const QString &userId, const QString &word);

    std::vector<QString> getUserSearchHistory (const QString &userId);
    std::vector<QString> getUserReciteHistory (const QString &userId);

    std::pair<int, int> getProgress (const QString &userId) const;

    std::vector<QString>
    getRandomWrongTranslations (const QString &correctTranslation,
                                int limit = 3);

private:
    explicit DbModel () : user_db (nullptr), dict_db (nullptr) { initDBs (); }

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

    static QString resolveResourcePath (const QString &relativePath);

    WordEntry parseCSVLineToWordEntry (const QString &csvLine);

    QStringList parseCSVLine (const QString &line);
};
