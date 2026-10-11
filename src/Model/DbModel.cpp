#include "DbModel.h"
#include "Controller/AiEventTap.h"
#include "Model/CacheModel.h"
#include "SQLiteCpp/Database.h"
#include "SQLiteCpp/Exception.h"
#include "SQLiteCpp/SQLiteCpp.h"
#include "SQLiteCpp/Statement.h"
#include "SQLiteCpp/Transaction.h"
#include "Utility/AspectGuard.h"
#include "Utility/Constants.h"
#include "Utility/Logging.h"
#include "Utility/PasswordHasher.h"
#include "Utility/Result.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QStringConverter>
#include <QTextStream>
#include <algorithm>
#include <atomic>
#include <random>
#include <stdexcept>
#include <thread>
#include <unordered_set>

using trance::aspect::guarded;
using trance::aspect::guardedRethrow;

struct DbModel::Impl
{
    struct DictionaryImportState
    {
        std::atomic_bool inProgress = false;
        std::atomic_bool ready = false;
    };

    std::unique_ptr<SQLite::Database> user_db;
    std::unique_ptr<SQLite::Database> dict_db;
    CacheModel<WordEntry> m_wordLookupCache;
    bool m_needsDictImport = false;
    std::shared_ptr<DictionaryImportState> m_dictImportState;

    Impl ()
        : user_db (nullptr), dict_db (nullptr),
          m_wordLookupCache (Constants::Settings::Cache::WORD_LOOKUP_MAX_BYTES),
          m_dictImportState (std::make_shared<DictionaryImportState> ())
    {
    }
};

DbModel::DbModel () : d (std::make_unique<Impl> ()) { initDBs (); }

DbModel::DbModel (DbModel &&) noexcept = default;

DbModel &DbModel::operator= (DbModel &&) noexcept = default;

DbModel::~DbModel () = default;

QString DbModel::getDbDir ()
{
    return QCoreApplication::applicationDirPath () + Constants::Paths::DB_DIR;
}

QString DbModel::getAvatarDir ()
{
    return QCoreApplication::applicationDirPath () +
           Constants::Paths::AVATAR_DIR;
}

void DbModel::initDBs ()
{
    QDir dir (getDbDir ());
    if (!dir.exists ())
    {
        dir.mkpath (".");
    }

    QString userDbPath = dir.filePath ("user.db");
    bool isNewUserDb = !QFile::exists (userDbPath);
    if (!isUserDbOpen ())
    {
        d->user_db = std::make_unique<SQLite::Database> (
            userDbPath.toStdString (),
            isNewUserDb ? (SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE)
                        : SQLite::OPEN_READWRITE);
    }

    if (d->user_db != nullptr)
    {
        initUserTable ();
    }

    QString dictDbPath = dir.filePath ("dict.db");
    bool isNewDictDb = !QFile::exists (dictDbPath);
    if (!isDictDbOpen ())
    {
        d->dict_db = std::make_unique<SQLite::Database> (
            dictDbPath.toStdString (),
            isNewDictDb ? (SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE)
                        : SQLite::OPEN_READWRITE);
    }

    if (d->dict_db != nullptr)
    {
        initDictTable ();
        if (isNewDictDb)
        {
            d->m_needsDictImport = true;
        }
    }

    if (d->dict_db != nullptr)
    {
        refreshDictImportRequirement ();
    }
}

void DbModel::initUserTable ()
{
    guarded (lcDb, "creating user table", [&]
    {
        d->user_db->exec ("PRAGMA journal_mode = WAL");
        d->user_db->exec ("PRAGMA synchronous = NORMAL");
        d->user_db->exec ("PRAGMA cache_size = 10000");
        d->user_db->exec ("PRAGMA temp_store = MEMORY");
        d->user_db->exec ("PRAGMA locking_mode = NORMAL");
        d->user_db->exec ("PRAGMA page_size = 4096");
        d->user_db->exec ("PRAGMA foreign_keys = ON");

        if (d->user_db != nullptr)
        {
            d->user_db->exec (
                "CREATE TABLE IF NOT EXISTS users("
                "user_id TEXT PRIMARY KEY,"
                "username TEXT NOT NULL UNIQUE,"
                "password_hash TEXT NOT NULL,"
                "email TEXT UNIQUE,"
                "avatar_path TEXT DEFAULT ':/image/DefaultUser.png');");

            d->user_db->exec (
                "CREATE TABLE IF NOT EXISTS user_search_history("
                "search_history_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "user_id TEXT NOT NULL,"
                "search_word TEXT NOT NULL,"
                "FOREIGN KEY(user_id) REFERENCES users(user_id) "
                "ON DELETE CASCADE "
                "ON UPDATE CASCADE );");

            d->user_db->exec (
                "CREATE TABLE IF NOT EXISTS user_recite_history("
                " recite_history_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                " user_id TEXT NOT NULL,"
                " recite_word TEXT NOT NULL,"
                " FOREIGN KEY(user_id) REFERENCES users(user_id) "
                "ON DELETE CASCADE ON UPDATE CASCADE );");

            d->user_db->exec ("CREATE TABLE IF NOT EXISTS user_progress("
                              "user_id TEXT PRIMARY KEY,"
                              "total_words INTEGER DEFAULT 15,"
                              "current_progress INTEGER DEFAULT 0 CHECK "
                              "(current_progress >= 0 AND current_progress <= "
                              "total_words), "
                              "FOREIGN KEY(user_id) REFERENCES users(user_id) "
                              "ON DELETE CASCADE ON UPDATE CASCADE);");

            d->user_db->exec (
                "CREATE TABLE IF NOT EXISTS user_favorites("
                "favorites_word_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "user_id TEXT NOT NULL,"
                "word TEXT NOT NULL,"
                "FOREIGN KEY(user_id) REFERENCES users(user_id) "
                "ON DELETE CASCADE ON UPDATE CASCADE );");

            d->user_db->exec ("CREATE TABLE IF NOT EXISTS user_vocabulary("
                              "user_id TEXT NOT NULL,"
                              "word TEXT NOT NULL,"
                              "status INTEGER DEFAULT -1 CHECK (status IN ( "
                              "0, 1)),"
                              "FOREIGN KEY(user_id) REFERENCES users(user_id) "
                              "ON DELETE CASCADE ON UPDATE CASCADE,"
                              "PRIMARY KEY (user_id, word));");

            d->user_db->exec (
                "CREATE TABLE IF NOT EXISTS ai_providers("
                "provider_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "user_id TEXT NOT NULL,"
                "name TEXT NOT NULL,"
                "base_url TEXT NOT NULL,"
                "api_key_ref TEXT DEFAULT '',"
                "chat_model TEXT NOT NULL,"
                "embedding_model TEXT DEFAULT '',"
                "is_active INTEGER DEFAULT 0 CHECK (is_active IN (0, 1)),"
                "FOREIGN KEY(user_id) REFERENCES users(user_id) "
                "ON DELETE CASCADE ON UPDATE CASCADE,"
                "UNIQUE (user_id, name));");

            SQLite::Transaction migration (*d->user_db);
            d->user_db->exec (
                "DELETE FROM user_favorites "
                "WHERE favorites_word_id NOT IN ("
                "SELECT favorite_id FROM ("
                "SELECT MIN(favorites_word_id) AS favorite_id "
                "FROM user_favorites GROUP BY user_id, word));");
            d->user_db->exec ("DROP INDEX IF EXISTS idx_user_favorites;");
            d->user_db->exec (
                "DROP INDEX IF EXISTS idx_user_search_history;");
            d->user_db->exec (
                "DROP INDEX IF EXISTS idx_user_recite_history;");
            d->user_db->exec (
                "CREATE UNIQUE INDEX IF NOT EXISTS idx_user_favorites ON "
                "user_favorites(user_id, word);");

            d->user_db->exec (
                "CREATE INDEX IF NOT EXISTS idx_user_search_history "
                "ON user_search_history(user_id, search_history_id DESC);");

            d->user_db->exec (
                "CREATE INDEX IF NOT EXISTS idx_user_recite_history "
                "ON user_recite_history(user_id, recite_history_id DESC);");

            d->user_db->exec (
                "CREATE INDEX IF NOT EXISTS idx_user_vocabulary_user_status "
                "ON user_vocabulary(user_id, status);");
            migration.commit ();
        }
    });
}

void DbModel::initDictTable ()
{
    guarded (lcDb, "creating dictionary table", [&]
    {
        d->dict_db->exec ("PRAGMA journal_mode = WAL");
        d->dict_db->exec ("PRAGMA synchronous = NORMAL");
        d->dict_db->exec ("PRAGMA cache_size = 20000");
        d->dict_db->exec ("PRAGMA temp_store = MEMORY");
        d->dict_db->exec ("PRAGMA locking_mode = NORMAL");
        d->dict_db->exec ("PRAGMA page_size = 8192");
        d->dict_db->exec ("PRAGMA foreign_keys = ON");

        if (d->dict_db != nullptr)
        {
            d->dict_db->exec ("CREATE TABLE IF NOT EXISTS users("
                              "user_id TEXT PRIMARY KEY,"
                              "username TEXT NOT NULL UNIQUE);");

            d->dict_db->exec (
                "CREATE TABLE IF NOT EXISTS words("
                "word_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "word TEXT NOT NULL UNIQUE,"
                "part_of_speech TEXT,"
                "pronunciation TEXT,"
                "frequency INTEGER DEFAULT 0,"
                "notes TEXT,"
                "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                "updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP);");

            d->dict_db->exec (
                "CREATE TABLE IF NOT EXISTS word_translations("
                "translation_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "source_word TEXT NOT NULL,"
                "target_word TEXT NOT NULL,"
                "source_language TEXT NOT NULL,"
                "target_language TEXT NOT NULL,"
                "confidence_score REAL DEFAULT 1.0 CHECK (confidence_score "
                ">= 0.0 AND confidence_score <= 1.0),"
                "UNIQUE(source_word,target_word,source_language,target_"
                "language));");

            d->dict_db->exec ("CREATE TABLE IF NOT EXISTS examples("
                              "example_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                              "word TEXT NOT NULL,"
                              "example_sentence TEXT NOT NULL,"
                              "translation TEXT,"
                              "FOREIGN KEY(word) REFERENCES words(word)"
                              "ON DELETE CASCADE ON UPDATE CASCADE);");

            d->dict_db->exec (
                "CREATE INDEX IF NOT EXISTS idx_words_word ON words(word);");
            d->dict_db->exec (
                "CREATE INDEX IF NOT EXISTS idx_translations_source ON "
                "word_translations(source_word, source_language);");
            d->dict_db->exec (
                "CREATE INDEX IF NOT EXISTS idx_translations_target ON "
                "word_translations(target_word, target_language);");
            d->dict_db->exec ("CREATE INDEX IF NOT EXISTS idx_examples_word ON "
                              "examples(word);");
        }
    });
}

bool DbModel::isUserDbOpen () const
{
    if (d->user_db)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool DbModel::isDictDbOpen () const
{
    if (d->dict_db)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool DbModel::isDictionaryImportInProgress () const
{
    return d->m_dictImportState != nullptr &&
           d->m_dictImportState->inProgress.load ();
}

bool DbModel::isDictionaryReady () const
{
    return d->m_dictImportState != nullptr &&
           d->m_dictImportState->ready.load ();
}

void DbModel::refreshDictImportRequirement ()
{
    if (!d->dict_db)
    {
        d->m_needsDictImport = true;
        if (d->m_dictImportState != nullptr)
        {
            d->m_dictImportState->ready.store (false);
        }
        return;
    }

    try
    {
        SQLite::Statement tableQuery (
            *d->dict_db, "SELECT COUNT(*) FROM sqlite_master "
                         "WHERE type = 'table' AND name = 'words'");
        tableQuery.executeStep ();

        if (tableQuery.getColumn (0).getInt () == 0)
        {
            initDictTable ();
            d->m_needsDictImport = true;
        }
        else
        {
            d->m_needsDictImport = countDictionaryWords (*d->dict_db) == 0;
        }

        if (d->m_dictImportState != nullptr)
        {
            d->m_dictImportState->ready.store (!d->m_needsDictImport);
        }
    }
    catch (const std::exception &e)
    {
        trance::log::logException (
            lcDb, "Error refreshing dictionary import state", e);
        d->m_needsDictImport = true;
        if (d->m_dictImportState != nullptr)
        {
            d->m_dictImportState->ready.store (false);
        }
    }
    catch (...)
    {
        trance::log::logExceptionUnknown (
            lcDb, "Error refreshing dictionary import state");
        d->m_needsDictImport = true;
        if (d->m_dictImportState != nullptr)
        {
            d->m_dictImportState->ready.store (false);
        }
    }
}

void DbModel::configureDictDatabasePragmas (SQLite::Database &database)
{
    database.exec ("PRAGMA journal_mode = WAL");
    database.exec ("PRAGMA synchronous = NORMAL");
    database.exec ("PRAGMA cache_size = 20000");
    database.exec ("PRAGMA temp_store = MEMORY");
    database.exec ("PRAGMA locking_mode = NORMAL");
    database.exec ("PRAGMA page_size = 8192");
    database.exec ("PRAGMA foreign_keys = ON");
}

int DbModel::countDictionaryWords (SQLite::Database &database)
{
    SQLite::Statement query (database, "SELECT COUNT(*) FROM words");
    query.executeStep ();
    return query.getColumn (0).getInt ();
}

void DbModel::insertWordBatchSyncInternal (SQLite::Database &database,
                                           const std::vector<WordEntry> &batch)
{
    if (batch.empty ())
    {
        return;
    }

    SQLite::Statement wordsStmt (
        database, "INSERT OR IGNORE INTO words "
                  "(word, part_of_speech, pronunciation, frequency, notes) "
                  "VALUES (?, ?, ?, ?, ?)");

    SQLite::Statement translationsStmt (
        database, "INSERT OR IGNORE INTO word_translations "
                  "(source_word, source_language, target_word, "
                  "target_language, confidence_score) "
                  "VALUES (?, ?, ?, ?, ?)");

    for (const auto &entry : batch)
    {
        wordsStmt.reset ();
        wordsStmt.bind (1, entry.word.toStdString ());
        wordsStmt.bind (2, entry.partOfSpeech.toStdString ());
        wordsStmt.bind (3, entry.pronunciation.toStdString ());
        wordsStmt.bind (4, entry.frequency);
        wordsStmt.bind (5, entry.notes.toStdString ());
        wordsStmt.exec ();

        if (!entry.translation.isEmpty ())
        {
            translationsStmt.reset ();
            translationsStmt.bind (1, entry.word.toStdString ());
            translationsStmt.bind (2, entry.language.toStdString ());
            translationsStmt.bind (3, entry.translation.toStdString ());
            translationsStmt.bind (4, "zh");
            translationsStmt.bind (5, 1.0);
            translationsStmt.exec ();
        }
    }
}

void DbModel::importFromFileSyncInternal (
    SQLite::Database &database, const QString &filePath,
    std::function<void (int, int)> progressCallback)
{
    QFile csvFile (filePath);

    if (!csvFile.open (QIODevice::ReadOnly | QIODevice::Text))
    {
        throw std::runtime_error ("File open failed");
    }

    QTextStream inputStream (&csvFile);
    inputStream.setEncoding (QStringConverter::Utf8);

    inputStream.readLine ();

    constexpr int batchSize = 1000;
    std::vector<WordEntry> batch;
    batch.reserve (batchSize);

    long long totalLines = 0;
    long long processedLines = 0;
    QTextStream countStream (&csvFile);
    countStream.seek (inputStream.pos ());
    while (!countStream.atEnd ())
    {
        countStream.readLine ();
        totalLines++;
    }

    inputStream.seek (0);
    inputStream.readLine ();

    SQLite::Transaction transaction (database);
    database.exec ("DELETE FROM examples");
    database.exec ("DELETE FROM word_translations");
    database.exec ("DELETE FROM words");

    while (!inputStream.atEnd ())
    {
        QString line = inputStream.readLine ().trimmed ();
        if (line.isEmpty ())
        {
            continue;
        }

        WordEntry entry = parseCSVLineToWordEntry (line);
        if (!entry.word.isEmpty ())
        {
            batch.emplace_back (std::move (entry));
        }

        processedLines++;

        if (batch.size () >= batchSize)
        {
            insertWordBatchSyncInternal (database, batch);
            batch.clear ();

            if (progressCallback)
            {
                progressCallback (static_cast<int> (processedLines),
                                  static_cast<int> (totalLines));
            }
        }
    }

    if (!batch.empty ())
    {
        insertWordBatchSyncInternal (database, batch);
    }

    transaction.commit ();

    if (progressCallback)
    {
        progressCallback (static_cast<int> (totalLines),
                          static_cast<int> (totalLines));
    }
}

bool DbModel::userExists (const QString &username) const
{
    if (!isUserDbOpen ())
    {
        qCWarning (lcDb) << "User database is not open";
        return false;
    }

    return guarded (lcDb, "checking if user exists", false, [&]
    {
        SQLite::Statement query (
            *d->user_db, "SELECT COUNT(*) FROM users WHERE username = ?");
        query.bind (1, username.toStdString ());
        query.executeStep ();
        return query.getColumn (0).getInt () > 0;
    });
}

bool DbModel::userIdExists (const QString &userId) const
{
    if (!isUserDbOpen ())
    {
        qCWarning (lcDb) << "User database is not open";
        return false;
    }

    return guarded (lcDb, "checking if user exists", false, [&]
    {
        SQLite::Statement query (
            *d->user_db, "SELECT COUNT(*) FROM users WHERE user_id = ?");
        query.bind (1, userId.toStdString ());
        query.executeStep ();
        return query.getColumn (0).getInt () > 0;
    });
}

RegisterUserResult DbModel::registerUser (const QString &username,
                                          const QString &passwordHash)
{
    DbModel &instance = DbModel::getInstance ();
    if (!instance.isUserDbOpen ())
    {
        return RegisterUserResult::DatabaseError;
    }
    if (instance.userExists (username))
    {
        return RegisterUserResult::UserAlreadyExists;
    }
    try
    {
        QString userId = QUuid::createUuid ().toString (QUuid::WithoutBraces);

        SQLite::Statement query (
            *(instance.d->user_db),
            "INSERT INTO users (user_id, username, password_hash) "
            "VALUES (?, ?, ?)");
        query.bind (1, userId.toStdString ());
        query.bind (2, username.toStdString ());
        query.bind (3, passwordHash.toStdString ());
        query.exec ();

        // Check if the user was inserted successfully
        if (query.getChanges () == 0)
        {
            return RegisterUserResult::DatabaseError;
        }

        return RegisterUserResult::Success;
    }
    catch (const SQLite::Exception &e)
    {
        trance::log::logException (lcDb, "Error inserting user into database",
                                   e);
        return RegisterUserResult::DatabaseError;
    }
    catch (const std::exception &e)
    {
        trance::log::logException (lcDb, "Error inserting user into database",
                                   e);
        return RegisterUserResult::UnknownError;
    }
    catch (...)
    {
        trance::log::logExceptionUnknown (lcDb,
                                          "Error inserting user into database");
        return RegisterUserResult::UnknownError;
    }
}

UserAuthResult DbModel::verifyUser (const QString &username,
                                    const QString &password) const
{

    try
    {
        if (!isUserDbOpen ())
        {
            return UserAuthResult::StorageError;
        }
        SQLite::Statement query (*d->user_db, "SELECT password_hash FROM users "
                                              "WHERE username = ?");
        query.bind (1, username.toStdString ());
        if (query.executeStep ())
        {
            const QString storedHash = QString::fromStdString (
                query.getColumn (0).getString ());

            if (PasswordHasher::verifyPassword (password, storedHash))
            {
                if (PasswordHasher::needsRehash (storedHash))
                {
                    try
                    {
                        SQLite::Statement updateQuery (
                            *d->user_db,
                            "UPDATE users SET password_hash = ? "
                            "WHERE username = ?");
                        updateQuery.bind (
                            1, PasswordHasher::hashPassword (password)
                                   .toStdString ());
                        updateQuery.bind (2, username.toStdString ());
                        updateQuery.exec ();
                    }
                    catch (const std::exception &e)
                    {
                        trance::log::logException (
                            lcDb, "Failed to upgrade legacy password hash", e);
                    }
                }
                return UserAuthResult::Success;
            }
            else
            {
                return UserAuthResult::IncorrectPassword;
            }
        }
        else
        {
            return UserAuthResult::UserNotFound;
        }
    }
    catch (const SQLite::Exception &e)
    {
        trance::log::logException (lcDb, "Error verifying user in database",
                                   e);
        return UserAuthResult::StorageError;
    }
    catch (const std::exception &e)
    {
        trance::log::logException (lcDb, "Error verifying user in database",
                                   e);
        return UserAuthResult::UnknownError;
    }
    catch (...)
    {
        trance::log::logExceptionUnknown (lcDb,
                                          "Error verifying user in database");
        return UserAuthResult::UnknownError;
    }
}

void DbModel::deleteUser (const QString &username)
{
    if (!isUserDbOpen ())
    {
        qCWarning (lcDb) << "User database is not open";
        return;
    }

    guarded (lcDb, "deleting user from database", [&]
    {
        SQLite::Statement query (*d->user_db,
                                 "DELETE FROM users WHERE username = ?");
        query.bind (1, username.toStdString ());
        query.exec ();
    });
}

ChangeResult DbModel::updateUserPassword (const QString &username,
                                          const QString &oldPassword,
                                          const QString &newPassword)
{
    if (!isUserDbOpen ())
    {
        return ChangeResult::DatabaseError;
    }

    if (username.isEmpty () || oldPassword.isEmpty () || newPassword.isEmpty ())
    {
        return ChangeResult::NullValue;
    }

    try
    {
        SQLite::Statement query (*d->user_db, "SELECT password_hash FROM users "
                                              "WHERE username = ?");
        query.bind (1, username.toStdString ());
        if (!query.executeStep ())
        {
            return ChangeResult::Password_OldIncorrect;
        }

        const QString storedHash = QString::fromStdString (
            query.getColumn (0).getString ());

        if (!PasswordHasher::verifyPassword (oldPassword, storedHash))
        {
            return ChangeResult::Password_OldIncorrect;
        }

        if (PasswordHasher::verifyPassword (newPassword, storedHash))
        {
            return ChangeResult::StillSame;
        }

        SQLite::Statement updateQuery (*d->user_db,
                                       "UPDATE users SET password_hash = ? "
                                       "WHERE username = ?");
        updateQuery.bind (1, PasswordHasher::hashPassword (newPassword)
                                 .toStdString ());
        updateQuery.bind (2, username.toStdString ());
        updateQuery.exec ();

        return ChangeResult::Success;
    }
    catch (const SQLite::Exception &e)
    {
        trance::log::logException (
            lcDb, "Error updating user password in database", e);
        return ChangeResult::DatabaseError;
    }
    catch (const std::exception &e)
    {
        trance::log::logException (
            lcDb, "Error updating user password in database", e);
        return ChangeResult::UnknownError;
    }
    catch (...)
    {
        trance::log::logExceptionUnknown (
            lcDb, "Error updating user password in database");
        return ChangeResult::UnknownError;
    }
}

ChangeResult DbModel::changeUsername (const QString &oldUsername,
                                      const QString &newUsername)
{
    if (!isUserDbOpen ())
    {
        return ChangeResult::DatabaseError;
    }

    if (oldUsername == newUsername)
    {
        return ChangeResult::StillSame; // No change needed
    }

    if (newUsername.isEmpty () || oldUsername.isEmpty ())
    {
        return ChangeResult::NullValue; // New username cannot be empty
    }

    if (userExists (newUsername))
    {
        return ChangeResult::AlreadyExists; // New username already exists
    }

    try
    {
        SQLite::Statement query (*d->user_db, "UPDATE users SET username = ? "
                                              "WHERE username = ?");

        query.bind (1, newUsername.toStdString ());
        query.bind (2, oldUsername.toStdString ());
        query.exec ();

        if (query.getChanges () < 0)
        {
            return ChangeResult::DatabaseError;
        }

        return ChangeResult::Success;
    }
    catch (const SQLite::Exception &e)
    {
        trance::log::logException (lcDb, "Error changing username in database",
                                   e);
        return ChangeResult::DatabaseError;
    }
    catch (const std::exception &e)
    {
        trance::log::logException (lcDb, "Error changing username in database",
                                   e);
        return ChangeResult::UnknownError;
    }
    catch (...)
    {
        trance::log::logExceptionUnknown (lcDb,
                                          "Error changing username in database");
        return ChangeResult::UnknownError;
    }
}

ChangeResult DbModel::changeEmail (const QString &username,
                                   const QString &oldEmail,
                                   const QString &newEmail)
{
    if (!isUserDbOpen ())
    {
        return ChangeResult::DatabaseError;
    }

    if (oldEmail == newEmail)
    {
        return ChangeResult::StillSame;
    }

    if (username.isEmpty () || newEmail.isEmpty ())
    {
        return ChangeResult::NullValue;
    }

    try
    {
        SQLite::Statement query (
            *d->user_db, "UPDATE users SET email = ? WHERE username = ?");
        query.bind (1, newEmail.toStdString ());
        query.bind (2, username.toStdString ());
        query.exec ();

        if (query.getChanges () == 0)
        {
            return ChangeResult::DatabaseError;
        }

        return ChangeResult::Success;
    }
    catch (const SQLite::Exception &e)
    {
        trance::log::logException (lcDb, "Error changing email in database", e);
        return ChangeResult::DatabaseError;
    }
    catch (const std::exception &e)
    {
        trance::log::logException (lcDb, "Error changing email in database", e);
        return ChangeResult::UnknownError;
    }
    catch (...)
    {
        trance::log::logExceptionUnknown (lcDb,
                                          "Error changing email in database");
        return ChangeResult::UnknownError;
    }
}

ChangeResult DbModel::changeAvatar (const QString &username,
                                    const QString &avatarPath)
{
    if (!isUserDbOpen ())
    {
        return ChangeResult::DatabaseError;
    }
    if (username.isEmpty () || avatarPath.isEmpty ())
    {
        return ChangeResult::NullValue;
    }
    if (!QFile::exists (avatarPath))
    {
        return ChangeResult::FileNotFound;
    }

    QDir avatarDir (getAvatarDir ());
    if (!avatarDir.exists ())
    {
        if (!avatarDir.mkpath ("."))
        {
            qCWarning (lcDb) << "Failed to create avatar directory";
            return ChangeResult::DatabaseError;
        }
    }

    try
    {
        SQLite::Statement query (
            *d->user_db, "UPDATE users SET avatar_path = ? WHERE username = ?");
        query.bind (1, avatarPath.toStdString ());
        query.bind (2, username.toStdString ());
        query.exec ();

        if (query.getChanges () == 0)
        {
            return ChangeResult::DatabaseError;
        }

        return ChangeResult::Success;
    }
    catch (const SQLite::Exception &e)
    {
        trance::log::logException (lcDb, "Error changing user avatar in database", e);
        return ChangeResult::DatabaseError;
    }
    catch (const std::exception &e)
    {
        trance::log::logException (lcDb, "Error changing user avatar in database", e);
        return ChangeResult::UnknownError;
    }
    catch (...)
    {
        trance::log::logExceptionUnknown (
            lcDb, "Error changing user avatar in database");
        return ChangeResult::UnknownError;
    }
}

ChangeResult DbModel::updateReciteProgress (int current, int total,
                                            const QString &userId)
{
    if (!isUserDbOpen ())
    {
        return ChangeResult::DatabaseError;
    }

    if (current < 0 || total <= 0 || current > total)
    {
        qCWarning (lcDb) << "Invalid progress values";
        return ChangeResult::InvalidInput;
    }

    if (userId.isEmpty ())
    {
        qCWarning (lcDb) << "User ID is empty";
        return ChangeResult::NullValue;
    }

    try
    {
        SQLite::Statement query (*d->user_db,
                                 "INSERT OR REPLACE INTO user_progress "
                                 "(user_id, current_progress, total_words) "
                                 "VALUES (?, ?, ?)");
        query.bind (1, userId.toStdString ());
        query.bind (2, current);
        query.bind (3, total);
        query.exec ();

        if (query.getChanges () == 0 && !userIdExists (userId))
        {
            qCWarning (lcDb) << "No changes made to user progress";
            return ChangeResult::DatabaseError;
        }

        return ChangeResult::Success;
    }
    catch (const SQLite::Exception &e)
    {
        trance::log::logException (
            lcDb, "Error updating recite progress in database", e);
        return ChangeResult::DatabaseError;
    }
    catch (const std::exception &e)
    {
        trance::log::logException (
            lcDb, "Error updating recite progress in database", e);
        return ChangeResult::UnknownError;
    }
    catch (...)
    {
        trance::log::logExceptionUnknown (
            lcDb, "Error updating recite progress in database");
        return ChangeResult::UnknownError;
    }
}

std::optional<QString> DbModel::getUserId (const QString &username) const
{
    if (!isUserDbOpen ())
    {
        qCWarning (lcDb) << "User database is not open";
        return std::nullopt;
    }

    if (username.isEmpty ())
    {
        qCWarning (lcDb) << "Username is empty";
        return std::nullopt;
    }

    return guarded (lcDb, "getting user ID from database",
                    std::optional<QString>{},
                    [&]() -> std::optional<QString>
    {
        SQLite::Statement query (
            *d->user_db, "SELECT user_id FROM users WHERE username = ?");
        query.bind (1, username.toStdString ());

        if (query.executeStep ())
        {
            return QString::fromStdString (query.getColumn (0).getString ());
        }

        return std::nullopt;
    });
}

std::optional<QString> DbModel::getUserName (const QString &userId) const
{
    if (!isUserDbOpen ())
    {
        qCWarning (lcDb) << "User database is not open";
        return std::nullopt;
    }

    if (userId.isEmpty ())
    {
        qCWarning (lcDb) << "User ID is empty";
        return std::nullopt;
    }

    return guarded (lcDb, "getting username from database",
                    std::optional<QString>{},
                    [&]() -> std::optional<QString>
    {
        SQLite::Statement query (
            *d->user_db, "SELECT username FROM users WHERE user_id = ?");
        query.bind (1, userId.toStdString ());

        if (query.executeStep ())
        {
            return QString::fromStdString (query.getColumn (0).getString ());
        }

        return std::nullopt;
    });
}

QString DbModel::getUserPasswordHash (const QString &username) const
{
    if (!isUserDbOpen ())
    {
        qCWarning (lcDb) << "User database is not open";
        return QString ();
    }

    if (username.isEmpty ())
    {
        qCWarning (lcDb) << "Username is empty";
        return QString ();
    }

    return guarded (lcDb, "getting user password hash from database",
                    QString{}, [&]() -> QString
    {
        SQLite::Statement query (
            *d->user_db, "SELECT password_hash FROM users WHERE username = ?");
        query.bind (1, username.toStdString ());

        if (query.executeStep ())
        {
            return QString::fromStdString (query.getColumn (0).getString ());
        }

        return QString ();
    });
}

QString DbModel::getUserEmail (const QString &username) const
{
    if (!isUserDbOpen ())
    {
        qCWarning (lcDb) << "User database is not open";
        return QString ();
    }

    if (username.isEmpty ())
    {
        qCWarning (lcDb) << "Username is empty";
        return QString ();
    }

    return guarded (lcDb, "getting user email from database",
                    QString{}, [&]() -> QString
    {
        SQLite::Statement query (*d->user_db,
                                 "SELECT email FROM users WHERE username = ?");
        query.bind (1, username.toStdString ());

        if (query.executeStep ())
        {
            // Check if the email column is NULL
            if (!query.getColumn (0).isNull ())
            {
                return QString::fromStdString (
                    query.getColumn (0).getString ());
            }
        }

        return QString (); // Return empty string if no email found or email is
                           // NULL
    });
}

QString DbModel::getUserAvatarPath (const QString &username) const
{
    const QString fallbackAvatar = QStringLiteral (":/image/DefaultUser.png");

    if (!isUserDbOpen ())
    {
        qCWarning (lcDb) << "User database is not open";
        return fallbackAvatar;
    }

    if (username.isEmpty ())
    {
        qCWarning (lcDb) << "Username is empty";
        return fallbackAvatar;
    }

    return guarded (lcDb, "getting user avatar path from database",
                    fallbackAvatar, [&]() -> QString
    {
        SQLite::Statement query (
            *d->user_db, "SELECT avatar_path FROM users WHERE username = ?");
        query.bind (1, username.toStdString ());

        if (query.executeStep ())
        {

            if (!query.getColumn (0).isNull ())
            {
                return QString::fromStdString (
                    query.getColumn (0).getString ());
            }
        }

        return fallbackAvatar;
    });
}

std::vector<AiProviderConfig>
DbModel::getAiProviders (const QString &userId)
{
    if (!isUserDbOpen ())
    {
        qCWarning (lcDb) << "User database is not open";
        return {};
    }

    return guarded (lcDb, "getting AI providers",
                    std::vector<AiProviderConfig>{},
                    [&]() -> std::vector<AiProviderConfig>
    {
        SQLite::Statement query (
            *d->user_db,
            "SELECT provider_id, name, base_url, api_key_ref, chat_model, "
            "embedding_model, is_active FROM ai_providers WHERE user_id = ? "
            "ORDER BY is_active DESC, provider_id ASC");
        query.bind (1, userId.toStdString ());

        std::vector<AiProviderConfig> providers;

        while (query.executeStep ())
        {
            AiProviderConfig provider;
            provider.id = query.getColumn (0).getInt64 ();
            provider.name = QString::fromStdString (
                query.getColumn (1).getString ());
            provider.baseUrl = QString::fromStdString (
                query.getColumn (2).getString ());
            provider.apiKeyRef = QString::fromStdString (
                query.getColumn (3).getString ());
            provider.chatModel = QString::fromStdString (
                query.getColumn (4).getString ());
            provider.embeddingModel = QString::fromStdString (
                query.getColumn (5).getString ());
            provider.isActive = query.getColumn (6).getInt () == 1;
            providers.push_back (provider);
        }

        return providers;
    });
}

std::optional<AiProviderConfig>
DbModel::getActiveAiProvider (const QString &userId)
{
    const auto providers = getAiProviders (userId);

    for (const auto &provider : providers)
    {
        if (provider.isActive)
        {
            return provider;
        }
    }

    return std::nullopt;
}

ChangeResult DbModel::upsertAiProvider (const QString &userId,
                                        AiProviderConfig &provider)
{
    if (!isUserDbOpen ())
    {
        qCWarning (lcDb) << "User database is not open";
        return ChangeResult::DatabaseError;
    }

    if (userId.isEmpty () || !provider.isValid ())
    {
        qCWarning (lcDb) << "Invalid AI provider configuration";
        return ChangeResult::InvalidInput;
    }

    return guarded (lcDb, "upserting AI provider",
                    ChangeResult::DatabaseError, [&]() -> ChangeResult
    {
        SQLite::Transaction transaction (*d->user_db);

        if (provider.id < 0)
        {
            SQLite::Statement query (
                *d->user_db,
                "INSERT INTO ai_providers (user_id, name, base_url, "
                "api_key_ref, chat_model, embedding_model, is_active) "
                "VALUES (?, ?, ?, ?, ?, ?, ?)");
            query.bind (1, userId.toStdString ());
            query.bind (2, provider.name.toStdString ());
            query.bind (3, provider.baseUrl.toStdString ());
            query.bind (4, provider.apiKeyRef.toStdString ());
            query.bind (5, provider.chatModel.toStdString ());
            query.bind (6, provider.embeddingModel.toStdString ());
            query.bind (7, provider.isActive ? 1 : 0);
            query.exec ();
            provider.id =
                static_cast<qlonglong> (d->user_db->getLastInsertRowid ());
        }
        else
        {
            SQLite::Statement query (
                *d->user_db,
                "UPDATE ai_providers SET name = ?, base_url = ?, "
                "api_key_ref = ?, chat_model = ?, embedding_model = ? "
                "WHERE provider_id = ? AND user_id = ?");
            query.bind (1, provider.name.toStdString ());
            query.bind (2, provider.baseUrl.toStdString ());
            query.bind (3, provider.apiKeyRef.toStdString ());
            query.bind (4, provider.chatModel.toStdString ());
            query.bind (5, provider.embeddingModel.toStdString ());
            query.bind (6, static_cast<int64_t> (provider.id));
            query.bind (7, userId.toStdString ());
            query.exec ();

            if (query.getChanges () == 0)
            {
                qCWarning (lcDb) << "AI provider not found for this user";
                return ChangeResult::InvalidInput;
            }
        }

        if (provider.isActive)
        {
            SQLite::Statement deactivate (*d->user_db,
                                          "UPDATE ai_providers SET is_active "
                                          "= 0 WHERE user_id = ? AND "
                                          "provider_id != ?");
            deactivate.bind (1, userId.toStdString ());
            deactivate.bind (
                2, static_cast<int64_t> (provider.id));
            deactivate.exec ();
        }

        transaction.commit ();
        return ChangeResult::Success;
    });
}

ChangeResult DbModel::deleteAiProvider (const QString &userId,
                                        qlonglong providerId)
{
    if (!isUserDbOpen ())
    {
        qCWarning (lcDb) << "User database is not open";
        return ChangeResult::DatabaseError;
    }

    return guarded (lcDb, "deleting AI provider",
                    ChangeResult::DatabaseError, [&]() -> ChangeResult
    {
        SQLite::Statement query (*d->user_db,
                                 "DELETE FROM ai_providers WHERE provider_id "
                                 "= ? AND user_id = ?");
        query.bind (1, static_cast<int64_t> (providerId));
        query.bind (2, userId.toStdString ());
        query.exec ();
        return ChangeResult::Success;
    });
}

ChangeResult DbModel::setActiveAiProvider (const QString &userId,
                                           qlonglong providerId)
{
    if (!isUserDbOpen ())
    {
        qCWarning (lcDb) << "User database is not open";
        return ChangeResult::DatabaseError;
    }

    return guarded (lcDb, "activating AI provider",
                    ChangeResult::DatabaseError, [&]() -> ChangeResult
    {
        SQLite::Transaction transaction (*d->user_db);

        SQLite::Statement deactivate (*d->user_db,
                                      "UPDATE ai_providers SET is_active = 0 "
                                      "WHERE user_id = ?");
        deactivate.bind (1, userId.toStdString ());
        deactivate.exec ();

        SQLite::Statement query (*d->user_db,
                                 "UPDATE ai_providers SET is_active = 1 "
                                 "WHERE provider_id = ? AND user_id = ?");
        query.bind (1, static_cast<int64_t> (providerId));
        query.bind (2, userId.toStdString ());
        query.exec ();

        if (query.getChanges () == 0)
        {
            qCWarning (lcDb) << "AI provider not found for this user";
            return ChangeResult::InvalidInput;
        }

        transaction.commit ();
        return ChangeResult::Success;
    });
}

AsyncTask<void> DbModel::importWordEntry (const WordEntry &wordEntry)
{
    if (!isDictDbOpen ())
    {
        qCWarning (lcDb) << "Dictionary database is not open";
        co_return;
    }

    try
    {

        // Insert word entry into the words table
        SQLite::Transaction transaction (*d->dict_db);

        SQLite::Statement wordsQuery (
            *d->dict_db, "INSERT INTO words "
                         "(word,part_of_speech,pronunciation,frequency,"
                         "notes) VALUES (?,?,?,?,?)");

        wordsQuery.bind (1, wordEntry.word.toStdString ());
        wordsQuery.bind (2, wordEntry.partOfSpeech.toStdString ());
        wordsQuery.bind (3, wordEntry.pronunciation.toStdString ());
        wordsQuery.bind (4, wordEntry.frequency);
        wordsQuery.bind (5, wordEntry.notes.toStdString ());
        wordsQuery.exec ();

        // Insert translations

        if (!wordEntry.translation.isEmpty ())
        {
            SQLite::Statement translationsQuery (
                *d->dict_db, "INSERT INTO word_translations"
                             "(source_word, source_language, "
                             "target_word, target_language,confidence_score) "
                             "VALUES (?,?,?,?,?)");
            translationsQuery.bind (1, wordEntry.word.toStdString ());
            translationsQuery.bind (2, wordEntry.language.toStdString ());
            translationsQuery.bind (3, wordEntry.translation.toStdString ());
            translationsQuery.bind (4, "zh");
            translationsQuery.bind (5, 1.0); // Default confidence score
            translationsQuery.exec ();
        }

        // Insert examples

        if (!wordEntry.examples.isEmpty ())
        {
            SQLite::Statement examplesQuery (*d->dict_db,
                                             "INSERT INTO examples (word, "
                                             "example_sentence,translation) "
                                             "VALUES (?,?,?)");
            examplesQuery.bind (1, wordEntry.word.toStdString ());
            examplesQuery.bind (2, wordEntry.examples.toStdString ());
            examplesQuery.bind (3, wordEntry.exampleTranslation.toStdString ());
            examplesQuery.exec ();
        }

        transaction.commit ();
        clearWordLookupCache ();
        co_return;
    }
    catch (const std::exception &e)
    {
        trance::log::logException (lcDb, "Error importing word entry", e);
    }
    catch (...)
    {
        trance::log::logExceptionUnknown (lcDb, "Error importing word entry");
    }
}

AsyncTask<void>
DbModel::importWordEntriesAsync (const std::vector<WordEntry> &wordEntries)
{
    if (!isDictDbOpen () || wordEntries.empty ())
    {
        qCWarning (lcDb) << "Dictionary database is not open or no word "
                            "entries provided";
        co_return;
    }

    try
    {
        SQLite::Transaction transaction (*d->dict_db);

        const int batchSize = 1000;
        for (size_t i = 0; i < wordEntries.size (); i += batchSize)
        {
            size_t endIdx = std::min (i + batchSize, wordEntries.size ());
            std::vector<WordEntry> batch (wordEntries.begin () + i,
                                          wordEntries.begin () + endIdx);

            co_await insertWordBatch (batch);

            // let go of the coroutine to allow other tasks to run
            if (i % (batchSize * 10) == 0)
            {
                co_await std::suspend_always{};
            }
        }

        transaction.commit ();
    }
    catch (const std::exception &e)
    {
        trance::log::logException (lcDb, "Error importing word entries", e);
    }
    catch (...)
    {
        trance::log::logExceptionUnknown (lcDb, "Error importing word entries");
    }
}

AsyncTask<void>
DbModel::importFromFileAsync (const QString &filePath,
                              std::function<void (int, int)> progressCallback)
{
    if (filePath.isEmpty ())
    {
        qCWarning (lcDb) << "Dictionary file path is empty";
        co_return;
    }

    const auto importState = d->m_dictImportState;
    if (importState == nullptr)
    {
        qCWarning (lcDb) << "Dictionary import state is not initialized";
        co_return;
    }

    if (importState->inProgress.exchange (true))
    {
        qCDebug (lcDb) << "Dictionary import is already running";
        co_return;
    }

    importState->ready.store (false);

    const QString dictDbPath = QDir (getDbDir ()).filePath ("dict.db");

    std::thread (
        [dictDbPath, filePath, progressCallback = std::move (progressCallback),
         importState] () mutable
        {
            try
            {
                SQLite::Database backgroundDb (dictDbPath.toStdString (),
                                               SQLite::OPEN_READWRITE);
                configureDictDatabasePragmas (backgroundDb);

                qCDebug (lcDb) << "Background dictionary import started from:"
                               << filePath;

                importFromFileSyncInternal (backgroundDb, filePath,
                                            std::move (progressCallback));

                importState->ready.store (countDictionaryWords (backgroundDb) >
                                          0);
                qCDebug (lcDb) << "Dictionary import completed!";
            }
            catch (const std::exception &e)
            {
                trance::log::logException (
                    lcDb, "Error importing from file in background", e);
                importState->ready.store (false);
            }
            catch (...)
            {
                trance::log::logExceptionUnknown (
                    lcDb, "Error importing from file in background");
                importState->ready.store (false);
            }

            importState->inProgress.store (false);
        })
        .detach ();

    co_return;
}

void DbModel::importFromFileSync (
    const QString &filePath, std::function<void (int, int)> progressCallback)
{
    if (!isDictDbOpen ())
    {
        qCWarning (lcDb) << "Dictionary database is not open";
        return;
    }

    guarded (lcDb, "importing from file", [&]
    {
        importFromFileSyncInternal (*d->dict_db, filePath, progressCallback);
        clearWordLookupCache ();
        if (d->m_dictImportState != nullptr)
        {
            d->m_dictImportState->ready.store (
                countDictionaryWords (*d->dict_db) > 0);
        }
    });
}

void DbModel::insertWordBatchSync (const std::vector<WordEntry> &batch)
{
    if (batch.empty ())
    {
        qCWarning (lcDb) << "Batch is empty";
        return;
    }

    guardedRethrow (lcDb, "batch insert", [&]
    {
        insertWordBatchSyncInternal (*d->dict_db, batch);
        clearWordLookupCache ();
    });
}

AsyncTask<void> DbModel::insertWordBatch (const std::vector<WordEntry> &batch)
{
    if (batch.empty ())
    {
        qCWarning (lcDb) << "Batch is empty";
        co_return;
    }

    guardedRethrow (lcDb, "batch insert", [&]
    {
        SQLite::Statement wordsStmt (
            *d->dict_db,
            "INSERT OR IGNORE INTO words "
            "(word, part_of_speech, pronunciation, frequency, notes) "
            "VALUES (?, ?, ?, ?, ?)");

        SQLite::Statement translationsStmt (
            *d->dict_db, "INSERT OR IGNORE INTO word_translations "
                         "(source_word, source_language, target_word, "
                         "target_language, confidence_score) "
                         "VALUES (?, ?, ?, ?, ?)");

        for (const auto &entry : batch)
        {

            wordsStmt.reset ();
            wordsStmt.bind (1, entry.word.toStdString ());
            wordsStmt.bind (2, entry.partOfSpeech.toStdString ());
            wordsStmt.bind (3, entry.pronunciation.toStdString ());
            wordsStmt.bind (4, entry.frequency);
            wordsStmt.bind (5, entry.notes.toStdString ());
            wordsStmt.exec ();

            if (!entry.translation.isEmpty ())
            {
                translationsStmt.reset ();
                translationsStmt.bind (1, entry.word.toStdString ());
                translationsStmt.bind (2, entry.language.toStdString ());
                translationsStmt.bind (3, entry.translation.toStdString ());
                translationsStmt.bind (4, "zh");
                translationsStmt.bind (5, 1.0);
                translationsStmt.exec ();
            }
        }

        clearWordLookupCache ();
    });

    co_return;
}

std::optional<WordEntry> DbModel::lookupWord (const QString &word,
                                              const QString &srcLang)
{
    if (!isDictDbOpen ())
    {
        qCWarning (lcDb) << "Dictionary database is not open";
        return std::nullopt;
    }

    const QString normalizedWord = word.trimmed ();
    const QString normalizedSrcLang = srcLang.trimmed ();

    if (normalizedWord.isEmpty () || normalizedSrcLang.isEmpty ())
    {
        qCWarning (lcDb) << "Word or source language is empty";
        return std::nullopt; // Invalid input
    }
    if (normalizedSrcLang != "en" && normalizedSrcLang != "zh")
    {
        qCWarning (lcDb) << "Unsupported source language";
        return std::nullopt; // Unsupported language
    }

    return guarded (lcDb, "looking up word in database",
                    std::optional<WordEntry>{},
                    [&]() -> std::optional<WordEntry>
    {
        const auto cacheKey =
            buildWordLookupCacheKey (normalizedWord, normalizedSrcLang);
        if (auto cachedEntry = d->m_wordLookupCache.get (cacheKey);
            cachedEntry.has_value ())
        {
            qCDebug (lcDb) << "Cache get:" << QString::fromStdString (cacheKey)
                           << "HIT";
            return cachedEntry;
        }
        qCDebug (lcDb) << "Cache get:" << QString::fromStdString (cacheKey)
                       << "MISS";

        WordEntry entry;
        bool found = false;
        if (normalizedSrcLang == "en")
        {
            SQLite::Statement queryWordBasic (*d->dict_db,
                                              "SELECT word, pronunciation "
                                              "FROM words WHERE word = ?");
            queryWordBasic.bind (1, normalizedWord.toStdString ());
            if (queryWordBasic.executeStep ())
            {
                found = true;
                entry.word = QString::fromStdString (
                    queryWordBasic.getColumn (0).getString ());
                entry.pronunciation = QString::fromStdString (
                    queryWordBasic.getColumn (1).getString ());
                entry.language = normalizedSrcLang;
            }

            SQLite::Statement queryTranslation (
                *d->dict_db, "SELECT target_word, target_language "
                             "FROM word_translations WHERE source_word = ? "
                             "AND source_language = ?");
            queryTranslation.bind (1, normalizedWord.toStdString ());
            queryTranslation.bind (2, normalizedSrcLang.toStdString ());
            if (queryTranslation.executeStep ())
            {
                entry.translation = QString::fromStdString (
                    queryTranslation.getColumn (0).getString ());
            }
            else
            {
                entry.translation = ""; // No translation found
            }
        }

        else if (normalizedSrcLang == "zh")
        {
            SQLite::Statement queryWord (
                *d->dict_db, "SELECT source_word FROM word_translations WHERE "
                             "target_word = ? AND target_language = ?");
            queryWord.bind (1, normalizedWord.toStdString ());
            queryWord.bind (2, normalizedSrcLang.toStdString ());
            if (queryWord.executeStep ())
            {
                found = true;
                entry.translation = normalizedWord;
                entry.word = QString::fromStdString (
                    queryWord.getColumn (0).getString ());
                entry.language = normalizedSrcLang;

                // get pronunciation
                SQLite::Statement queryPronunciation (
                    *d->dict_db,
                    "SELECT pronunciation FROM words WHERE word = ?");
                queryPronunciation.bind (1, entry.word.toStdString ());
                if (queryPronunciation.executeStep ())
                {
                    entry.pronunciation = QString::fromStdString (
                        queryPronunciation.getColumn (0).getString ());
                }
                else
                {
                    entry.pronunciation = ""; // No pronunciation found
                }
            }
        }

        if (!found || entry.word.isEmpty ())
        {
            return std::nullopt;
        }

        d->m_wordLookupCache.put (
            cacheKey, entry,
            std::chrono::minutes (
                Constants::Settings::Cache::WORD_LOOKUP_TTL_MINUTES));

        qCDebug (lcDb) << "Cache size after put:"
                       << d->m_wordLookupCache.sizeBytes () << "bytes";

        return entry;
    });
}

void DbModel::clearWordLookupCache () { d->m_wordLookupCache.clear (); }

std::size_t DbModel::getWordLookupCacheSizeBytes () const
{
    return d->m_wordLookupCache.sizeBytes ();
}

std::string DbModel::buildWordLookupCacheKey (const QString &word,
                                              const QString &srcLang)
{
    return QStringLiteral ("%1:{%2}").arg (srcLang, word).toStdString ();
}

std::vector<WordEntry> DbModel::searchWords (const QString &pattern,
                                             const QString &srcLang, int limit)
{
    if (!isDictDbOpen ())
    {
        qCWarning (lcDb) << "Dictionary database is not open";
        return std::vector<WordEntry> ();
    }
    if (pattern.isEmpty () || srcLang.isEmpty ())
    {
        qCWarning (lcDb) << "Pattern or source language is empty";
        return std::vector<WordEntry> (); // Invalid input
    }

    if (srcLang != "en" && srcLang != "zh")
    {
        qCWarning (lcDb) << "Unsupported source language";
        return std::vector<WordEntry> (); // Unsupported language
    }

    if (limit <= 0)
    {
        qCWarning (lcDb) << "Limit must be greater than 0";
        return std::vector<WordEntry> (); // Invalid limit
    }

    return guarded (lcDb, "searching words in database",
                    std::vector<WordEntry>{},
                    [&]() -> std::vector<WordEntry>
    {

        std::vector<WordEntry> results;
        if (srcLang == "en")
        {
            SQLite::Statement queryWordsBasic (
                *d->dict_db,
                "SELECT w.word, w.pronunciation, COALESCE(wt.target_word, ''), "
                "CASE"
                " WHEN w.word = ? THEN 1 "    // precise match
                " WHEN w.word LIKE ? THEN 2 " // prefix match
                " WHEN w.word LIKE ? THEN 3 " // include match
                " ELSE 4 "
                " END AS match_priority "
                " FROM words w "
                " LEFT JOIN ("
                " SELECT source_word, MIN(translation_id) AS translation_id "
                " FROM word_translations "
                " WHERE source_language = ? "
                " GROUP BY source_word"
                " ) first_translation "
                " ON first_translation.source_word = w.word "
                " LEFT JOIN word_translations wt "
                " ON wt.translation_id = first_translation.translation_id "
                " WHERE w.word = ? OR "
                " w.word LIKE ? OR "
                " w.word LIKE ? "
                " ORDER BY match_priority, w.word "
                " LIMIT ? ; ");

            queryWordsBasic.bind (1, srcLang.toStdString ());
            queryWordsBasic.bind (2, pattern.toStdString ());
            queryWordsBasic.bind (3, pattern.toStdString () + "%");
            queryWordsBasic.bind (4, "%" + pattern.toStdString () + "%");
            queryWordsBasic.bind (5, pattern.toStdString ());
            queryWordsBasic.bind (6, pattern.toStdString () + "%");
            queryWordsBasic.bind (7, "%" + pattern.toStdString () + "%");
            queryWordsBasic.bind (8, limit);

            while (queryWordsBasic.executeStep ())
            {
                WordEntry entry;
                entry.word = QString::fromStdString (
                    queryWordsBasic.getColumn (0).getString ());
                entry.pronunciation = QString::fromStdString (
                    queryWordsBasic.getColumn (1).getString ());
                entry.translation = QString::fromStdString (
                    queryWordsBasic.getColumn (2).getString ());
                entry.language = srcLang;

                results.emplace_back (entry);
            }
        }
        else if (srcLang == "zh")
        {
            SQLite::Statement queryTranslations (
                *d->dict_db,
                "SELECT wt.source_word, wt.target_word, "
                "COALESCE(w.pronunciation, ''), "
                "CASE"
                " WHEN wt.target_word = ? THEN 1 " // precise match
                " WHEN wt.target_word LIKE '_. %' AND wt.target_word LIKE ? "
                "THEN "
                "2 " // single letter+dot+space prefix (e.g., "a. xxx")
                " WHEN wt.target_word LIKE '__. %' AND wt.target_word LIKE ? "
                "THEN 3 " // two letters+dot+space prefix (e.g., "ad. xxx")
                " WHEN wt.target_word LIKE '___. %' AND wt.target_word LIKE ? "
                "THEN 4 " // three letters+dot+space prefix (e.g., "adj. xxx")
                " WHEN wt.target_word LIKE '. %' AND wt.target_word LIKE ? "
                "THEN "
                "5 " // dot+space prefix (e.g., ". xxx")
                " WHEN wt.target_word LIKE ? THEN 6 " // normal prefix match
                " WHEN wt.target_word LIKE ? THEN 7 " // include match
                " ELSE 8 "
                " END AS match_priority "
                " FROM word_translations wt "
                " LEFT JOIN words w ON w.word = wt.source_word "
                " WHERE wt.target_language = 'zh' AND ("
                " wt.target_word = ? OR "
                " (wt.target_word LIKE '_. %' AND wt.target_word LIKE ?) OR "
                " (wt.target_word LIKE '__. %' AND wt.target_word LIKE ?) OR "
                " (wt.target_word LIKE '___. %' AND wt.target_word LIKE ?) OR "
                " (wt.target_word LIKE '. %' AND wt.target_word LIKE ?) OR "
                " wt.target_word LIKE ? OR "
                " wt.target_word LIKE ? )"
                " ORDER BY match_priority, wt.target_word "
                " LIMIT ? ; ");

            queryTranslations.bind (1, pattern.toStdString ());
            queryTranslations.bind (2, "_. " + pattern.toStdString () + "%");
            queryTranslations.bind (3, "__. " + pattern.toStdString () + "%");
            queryTranslations.bind (4, "___. " + pattern.toStdString () + "%");
            queryTranslations.bind (5, ". " + pattern.toStdString () + "%");
            queryTranslations.bind (6, pattern.toStdString () + "%");
            queryTranslations.bind (7, "%" + pattern.toStdString () + "%");
            queryTranslations.bind (8, pattern.toStdString ());
            queryTranslations.bind (9, "_. " + pattern.toStdString () + "%");
            queryTranslations.bind (10, "__. " + pattern.toStdString () + "%");
            queryTranslations.bind (11, "___. " + pattern.toStdString () + "%");
            queryTranslations.bind (12, ". " + pattern.toStdString () + "%");
            queryTranslations.bind (13, pattern.toStdString () + "%");
            queryTranslations.bind (14, "%" + pattern.toStdString () + "%");
            queryTranslations.bind (15, limit);

            while (queryTranslations.executeStep ())
            {
                WordEntry entry;
                entry.word = QString::fromStdString (
                    queryTranslations.getColumn (0).getString ());
                entry.translation = QString::fromStdString (
                    queryTranslations.getColumn (1).getString ());
                entry.pronunciation = QString::fromStdString (
                    queryTranslations.getColumn (2).getString ());
                entry.language = srcLang;

                results.emplace_back (entry);
            }
        }

        return results;
    });
}

void DbModel::addToUserFavorites (const QString &userId, const QString &word)
{
    if (!isUserDbOpen ())
    {
        qCWarning (lcDb) << "User database is not open";
        return;
    }

    guarded (lcDb, "adding word to favorites", [&]
    {
        SQLite::Statement query (
            *d->user_db,
            "INSERT OR IGNORE INTO user_favorites (user_id, word) "
            "VALUES (?, ?)");
        query.bind (1, userId.toStdString ());
        query.bind (2, word.toStdString ());
        query.exec ();

        if (query.getChanges () > 0)
        {
            AiEventTap::notifyFavoriteChanged (word, true);
        }
    });
}

void DbModel::removeFromUserFavorites (const QString &userId,
                                       const QString &word)
{
    if (!isUserDbOpen ())
    {
        qCWarning (lcDb) << "User database is not open";
        return;
    }

    guarded (lcDb, "removing word from favorites", [&]
    {
        SQLite::Statement query (
            *d->user_db,
            "DELETE FROM user_favorites WHERE user_id = ? AND word = ?");
        query.bind (1, userId.toStdString ());
        query.bind (2, word.toStdString ());
        query.exec ();

        if (query.getChanges () > 0)
        {
            AiEventTap::notifyFavoriteChanged (word, false);
        }
    });
}

bool DbModel::isWordFavorited (const QString &userId, const QString &word) const
{
    if (!isUserDbOpen ())
    {
        qCWarning (lcDb) << "User database is not open";
        return false;
    }

    return guarded (lcDb, "checking if word is favorited", false, [&]
    {
        SQLite::Statement query (*d->user_db,
                                 "SELECT COUNT(*) FROM user_favorites WHERE "
                                 "user_id = ? AND word = ?");
        query.bind (1, userId.toStdString ());
        query.bind (2, word.toStdString ());
        query.executeStep ();
        return query.getColumn (0).getInt () > 0;
    });
}

bool DbModel::existsInMastered (const QString &userId,
                                const QString &word) const
{
    if (!isUserDbOpen ())
    {
        qCWarning (lcDb) << "User database is not open";
        return false;
    }

    return guarded (lcDb, "checking if word is mastered", false, [&]
    {
        SQLite::Statement query (*d->user_db,
                                 "SELECT COUNT(*) FROM user_vocabulary WHERE "
                                 "user_id = ? AND word = ? AND status = 1");
        query.bind (1, userId.toStdString ());
        query.bind (2, word.toStdString ());
        query.executeStep ();
        return query.getColumn (0).getInt () > 0;
    });
}

ChangeResult DbModel::updateWordStatus (const QString &userId,
                                        const QString &word, int status)
{
    // status: 0=learning, 1=mastered

    if (!isUserDbOpen ())
    {
        qCWarning (lcDb) << "User database is not open";
        return ChangeResult::DatabaseError;
    }

    if (userId.isEmpty () || word.isEmpty ())
    {
        qCWarning (lcDb) << "User ID or word is empty";
        return ChangeResult::InvalidInput;
    }

    if (status < 0 || status > 1)
    {
        qCWarning (lcDb) << "Invalid status value";
        return ChangeResult::InvalidInput;
    }

    try
    {
        SQLite::Statement query (
            *d->user_db,
            "INSERT OR REPLACE INTO "
            "user_vocabulary (user_id, word, status) VALUES (?, ?, ?)");
        query.bind (1, userId.toStdString ());
        query.bind (2, word.toStdString ());
        query.bind (3, status);
        query.exec ();

        if (query.getChanges () == 0)
        {
            qCWarning (lcDb) << "No changes made to user vocabulary";
            return ChangeResult::StillSame;
        }

        AiEventTap::notifyWordStatusChanged (word, status);

        return ChangeResult::Success;
    }
    catch (const SQLite::Exception &e)
    {

        trance::log::logException (
            lcDb, "Error updating word status in database", e);
        return ChangeResult::DatabaseError;
    }
    catch (const std::exception &e)
    {
        trance::log::logException (
            lcDb, "Error updating word status in database", e);
        return ChangeResult::UnknownError;
    }
    catch (...)
    {
        trance::log::logExceptionUnknown (
            lcDb, "Error updating word status in database");
        return ChangeResult::UnknownError;
    }
}

std::vector<QString> DbModel::getUserVocabulary (const QString &userId,
                                                 int status) // -1 means all
{

    if (!isUserDbOpen ())
    {
        qCWarning (lcDb) << "User database is not open";
        return std::vector<QString> ();
    }

    if (userId.isEmpty ())
    {
        qCWarning (lcDb) << "User ID is empty";
        return std::vector<QString> (); // Invalid input
    }

    if (status < -1 || status > 1)
    {
        qCWarning (lcDb) << "Invalid status value";
        return std::vector<QString> (); // Invalid status
    }

    return guarded (lcDb, "getting user vocabulary from database",
                    std::vector<QString>{}, [&]() -> std::vector<QString>
    {
        std::vector<QString> vocabularies;

        if (status == -1) // Get all
        {
            SQLite::Statement query (
                *d->user_db,
                "SELECT word, status FROM user_vocabulary WHERE user_id = ?");
            query.bind (1, userId.toStdString ());

            while (query.executeStep ())
            {
                QString word =
                    QString::fromStdString (query.getColumn (0).getString ());
                int wordStatus = query.getColumn (1).getInt ();

                if (wordStatus == 0 ||
                    wordStatus == 1) // Only learning or mastered
                {
                    vocabularies.emplace_back (word);
                }
            }
            return vocabularies;
        }
        else
        {
            SQLite::Statement query (
                *d->user_db,
                "SELECT word FROM user_vocabulary WHERE user_id = ? AND "
                "status = ?");
            query.bind (1, userId.toStdString ());
            query.bind (2, status);
            while (query.executeStep ())
            {
                QString word =
                    QString::fromStdString (query.getColumn (0).getString ());
                vocabularies.emplace_back (word);
            }
            return vocabularies;
        }
    });
}

std::vector<QString> DbModel::getUserFavorites (const QString &userId)
{
    if (!isUserDbOpen ())
    {
        qCWarning (lcDb) << "User database is not open";
        return std::vector<QString> ();
    }

    if (userId.isEmpty ())
    {
        qCWarning (lcDb) << "User ID is empty";
        return std::vector<QString> (); // Invalid input
    }

    return guarded (lcDb, "getting user favorites from database",
                    std::vector<QString>{}, [&]() -> std::vector<QString>
    {
        std::vector<QString> favorites;
        SQLite::Statement query (
            *d->user_db, "SELECT word FROM user_favorites WHERE user_id = ?");
        query.bind (1, userId.toStdString ());

        while (query.executeStep ())
        {
            QString word =
                QString::fromStdString (query.getColumn (0).getString ());
            favorites.emplace_back (word);
        }
        return favorites;
    });
}

std::optional<WordEntry> DbModel::getRandomWord ()
{
    if (!isDictDbOpen ())
    {
        qCWarning (lcDb) << "Dictionary database is not open";
        return std::nullopt;
    }

    return guarded (lcDb, "getting random word from database",
                    std::optional<WordEntry>{},
                    [&]() -> std::optional<WordEntry>
    {
        WordEntry randomWordEntry;

        SQLite::Statement countQuery (*d->dict_db,
                                      "SELECT COUNT(*) FROM words");
        if (!countQuery.executeStep ())
        {
            return std::nullopt;
        }

        int wordCount = countQuery.getColumn (0).getInt ();
        if (wordCount <= 0)
        {
            return std::nullopt;
        }

        std::random_device rd;
        std::mt19937 gen (rd ());
        std::uniform_int_distribution<> dis (0, wordCount - 1);
        int randomOffset = dis (gen);

        SQLite::Statement queryRandomWord (
            *d->dict_db,
            "SELECT w.word, w.pronunciation, COALESCE(wt.target_word, '') "
            "FROM words w "
            "LEFT JOIN ("
            "SELECT source_word, MIN(translation_id) AS translation_id "
            "FROM word_translations "
            "WHERE source_language = 'en' "
            "GROUP BY source_word"
            ") first_translation "
            "ON first_translation.source_word = w.word "
            "LEFT JOIN word_translations wt "
            "ON wt.translation_id = first_translation.translation_id "
            "ORDER BY w.word_id LIMIT 1 OFFSET ?");
        queryRandomWord.bind (1, randomOffset);

        if (queryRandomWord.executeStep ())
        {
            randomWordEntry.word = QString::fromStdString (
                queryRandomWord.getColumn (0).getString ());
            randomWordEntry.pronunciation = QString::fromStdString (
                queryRandomWord.getColumn (1).getString ());
            randomWordEntry.translation = QString::fromStdString (
                queryRandomWord.getColumn (2).getString ());
            randomWordEntry.language = "en"; // Default language
        }
        else
        {
            return std::nullopt;
        }

        return randomWordEntry;
    });
}

QString DbModel::resolveResourcePath (const QString &relativePath)
{
    QString normalized = relativePath;
    if (normalized.startsWith ('/'))
    {
        normalized = normalized.mid (1);
    }

    QDir dir (QCoreApplication::applicationDirPath ());
    constexpr int maxLevels = 6;
    for (int i = 0; i < maxLevels; ++i)
    {
        const QString candidate = QDir::cleanPath (dir.filePath (normalized));
        if (QFile::exists (candidate))
        {
            return candidate;
        }
        if (!dir.cdUp ())
        {
            break;
        }
    }

    return QDir::cleanPath (
        QDir (QCoreApplication::applicationDirPath ()).filePath (normalized));
}

WordEntry DbModel::parseCSVLineToWordEntry (const QString &csvLine)
{
    WordEntry entry;
    QStringList fields = parseCSVLine (csvLine);
    if (fields.size () < 4)
    {
        return entry; // Need at least word, phonetic, definition,
                      // translation
    }

    // CSV format:
    // word,phonetic,definition,translation,pos,collins,oxford,tag,bnc,frq,exchange,detail,audio
    entry.word = fields[0].trimmed ();
    entry.pronunciation = fields.size () > 1 ? fields[1].trimmed () : "";
    entry.translation = fields.size () > 3 ? fields[3].trimmed () : "";
    entry.partOfSpeech = fields.size () > 4 ? fields[4].trimmed () : "";
    entry.language = "en";

    // frq is at index 9
    if (fields.size () > 9)
    {
        bool ok;
        int freq = fields[9].toInt (&ok);
        entry.frequency = ok ? freq : 0;
    }
    else
    {
        entry.frequency = 0;
    }

    return entry;
}

QStringList DbModel::parseCSVLine (const QString &line)
{
    QStringList result;
    QString current;
    bool inQuotes = false;

    for (int i = 0; i < line.length (); ++i)
    {
        QChar c = line[i];

        if (c == '"')
        {
            if (inQuotes && i + 1 < line.length () && line[i + 1] == '"')
            {
                current += '"';
                ++i;
            }
            else
            {
                inQuotes = !inQuotes;
            }
        }
        else if (c == ',' && !inQuotes)
        {
            result << current;
            current.clear ();
        }
        else
        {
            current += c;
        }
    }

    result << current;
    return result;
}

AsyncTask<void> DbModel::initializeAsync (const QString &dictPath)
{
    initDBs ();

    if (!isDictDbOpen () || isDictionaryImportInProgress ())
    {
        co_return;
    }

    QString dictFilePath = dictPath;
    if (dictFilePath.isEmpty ())
    {
        dictFilePath =
            resolveResourcePath (Constants::Database::ECDICT_CSV_PATH);
    }

    try
    {
        const int wordCount = countDictionaryWords (*d->dict_db);
        d->m_needsDictImport = wordCount == 0;

        if (wordCount == 0 && QFile::exists (dictFilePath))
        {
            qCDebug (lcDb)
                << "Dictionary is empty, starting background import from:"
                << dictFilePath;

            importFromFileAsync (
                dictFilePath,
                [] (int current, int total)
                {
                    if (total > 0 && (current % 5000 == 0 || current == total))
                    {
                        qCDebug (lcDb) << "Dictionary import progress:"
                                       << (current * 100 / total) << "%";
                    }
                });
        }
        else if (wordCount > 0)
        {
            if (d->m_dictImportState != nullptr)
            {
                d->m_dictImportState->ready.store (true);
            }
            qCDebug (lcDb) << "Dictionary already contains" << wordCount
                           << "words";
        }
        else if (!QFile::exists (dictFilePath))
        {
            qCWarning (lcDb) << "Dictionary file not found at:"
                             << dictFilePath;
        }
    }
    catch (const std::exception &e)
    {
        trance::log::logException (lcDb, "Error checking/importing dictionary",
                                   e);
    }
    catch (...)
    {
        trance::log::logExceptionUnknown (lcDb,
                                          "Error checking/importing dictionary");
    }

    co_return;
}

void DbModel::addToSearchHistory (const QString &userId, const QString &word)
{
    if (!isUserDbOpen ())
    {
        qCWarning (lcDb) << "User database is not open";
        return;
    }

    guarded (lcDb, "adding to search history", [&]
    {
        SQLite::Statement query (*d->user_db,
                                 "INSERT INTO user_search_history (user_id, "
                                 "search_word) VALUES (?, ?)");
        query.bind (1, userId.toStdString ());
        query.bind (2, word.toStdString ());
        query.exec ();

        AiEventTap::notifyWordLookedUp (word);
    });
}

void DbModel::removeFromSearchHistory (const QString &userId,
                                       const QString &word)
{
    if (!isUserDbOpen ())
    {
        qCWarning (lcDb) << "User database is not open";
        return;
    }

    guarded (lcDb, "removing from search history", [&]
    {
        SQLite::Statement query (
            *d->user_db, "DELETE FROM user_search_history WHERE user_id = ? "
                         "AND search_word = ?");
        query.bind (1, userId.toStdString ());
        query.bind (2, word.toStdString ());
        query.exec ();
    });
}

void DbModel::addToReciteHistory (const QString &userId, const QString &word)
{
    if (!isUserDbOpen ())
    {
        qCWarning (lcDb) << "User database is not open";
        return;
    }

    guarded (lcDb, "adding to recite history", [&]
    {
        SQLite::Statement query (
            *d->user_db,
            "INSERT INTO user_recite_history (user_id, recite_word) "
            "VALUES (?, ?)");
        query.bind (1, userId.toStdString ());
        query.bind (2, word.toStdString ());
        query.exec ();

        AiEventTap::notifyWordRecited (word);
    });
}

void DbModel::removeFromReciteHistory (const QString &userId,
                                       const QString &word)
{
    if (!isUserDbOpen ())
    {
        qCWarning (lcDb) << "User database is not open";
        return;
    }

    guarded (lcDb, "removing from recite history", [&]
    {
        SQLite::Statement query (
            *d->user_db, "DELETE FROM user_recite_history WHERE user_id = ? "
                         "AND recite_word = ?");
        query.bind (1, userId.toStdString ());
        query.bind (2, word.toStdString ());
        query.exec ();
    });
}

std::vector<QString> DbModel::getUserSearchHistory (const QString &userId)
{
    std::vector<QString> history;

    if (!isUserDbOpen ())
    {
        qCWarning (lcDb) << "User database is not open";
        return history;
    }

    try
    {
        std::unordered_set<std::string> seenWords;
        SQLite::Statement query (
            *d->user_db, "SELECT search_word FROM user_search_history "
                         "WHERE user_id = ? ORDER BY search_history_id DESC");
        query.bind (1, userId.toStdString ());

        while (query.executeStep ())
        {
            std::string historyWord = query.getColumn (0).getString ();
            if (seenWords.insert (historyWord).second)
            {
                history.emplace_back (QString::fromStdString (historyWord));
            }
        }
    }
    catch (const std::exception &e)
    {
        trance::log::logException (lcDb, "Error getting user search history",
                                   e);
    }
    catch (...)
    {
        trance::log::logExceptionUnknown (lcDb,
                                          "Error getting user search history");
    }

    return history;
}

std::vector<QString> DbModel::getUserReciteHistory (const QString &userId)
{
    std::vector<QString> history;

    if (!isUserDbOpen ())
    {
        qCWarning (lcDb) << "User database is not open";
        return history;
    }

    try
    {
        std::unordered_set<std::string> seenWords;
        SQLite::Statement query (
            *d->user_db, "SELECT recite_word FROM user_recite_history "
                         "WHERE user_id = ? ORDER BY recite_history_id DESC");
        query.bind (1, userId.toStdString ());

        while (query.executeStep ())
        {
            std::string historyWord = query.getColumn (0).getString ();
            if (seenWords.insert (historyWord).second)
            {
                history.emplace_back (QString::fromStdString (historyWord));
            }
        }
    }
    catch (const std::exception &e)
    {
        trance::log::logException (lcDb, "Error getting user recite history",
                                   e);
    }
    catch (...)
    {
        trance::log::logExceptionUnknown (lcDb,
                                          "Error getting user recite history");
    }

    return history;
}

std::pair<int, int> DbModel::getProgress (const QString &userId) const
{

    int currentProgress = 0;
    int totalWords = 15;

    if (!isUserDbOpen ())
    {
        qCWarning (lcDb) << "User database is not open";
        return {0, 15};
    }

    try
    {
        SQLite::Statement query (
            *d->user_db,
            "SELECT current_progress,total_words FROM user_progress "
            "WHERE user_id = ?");

        query.bind (1, userId.toStdString ());

        if (query.executeStep ())
        {
            currentProgress = query.getColumn (0).getInt ();
            totalWords = query.getColumn (1).getInt ();

            return {currentProgress, totalWords};
        }

        return {currentProgress, totalWords};
    }
    catch (const std::exception &e)
    {

        trance::log::logException (lcDb, "Error getting user progress", e);
        return {currentProgress, totalWords};
    }
    catch (...)
    {
        trance::log::logExceptionUnknown (lcDb, "Error getting user progress");
        return {currentProgress, totalWords};
    }
}

std::vector<QString>
DbModel::getRandomWrongTranslations (const QString &correctTranslation,
                                     int limit)
{
    std::vector<QString> wrongTranslations;
    if (!isDictDbOpen ())
    {
        qCWarning (lcDb) << "Dictionary database is not open";
        return std::vector<QString> ();
    }

    if (correctTranslation.isEmpty () || limit <= 0)
    {
        qCWarning (lcDb) << "Invalid input for getting wrong translations";
        return std::vector<QString> ();
    }

    return guarded (lcDb, "preparing query for wrong translations",
                    std::vector<QString>{}, [&]() -> std::vector<QString>
    {
        SQLite::Statement countQuery (
            *d->dict_db,
            "SELECT COUNT(*) FROM ("
            "SELECT target_word FROM word_translations "
            "WHERE target_word != ? GROUP BY target_word)");
        countQuery.bind (1, correctTranslation.toStdString ());
        countQuery.executeStep ();
        int candidateCount = countQuery.getColumn (0).getInt ();

        if (candidateCount <= 0)
        {
            qCWarning (lcDb) << "No translations found in the database";
            return std::vector<QString> ();
        }

        std::random_device rd;
        std::mt19937 gen (rd ());
        int candidatePoolSize = std::min (candidateCount, std::max (limit * 4,
                                                                    limit + 8));
        int maxOffset = std::max (0, candidateCount - candidatePoolSize);
        std::uniform_int_distribution<> dis (0, maxOffset);
        int randomOffset = dis (gen);
        std::unordered_set<std::string> seenTranslations;

        auto appendCandidates = [&] (int offset)
        {
            SQLite::Statement translationQuery (
                *d->dict_db,
                "SELECT target_word, MIN(translation_id) AS first_id "
                "FROM word_translations "
                "WHERE target_word != ? "
                "GROUP BY target_word "
                "ORDER BY first_id LIMIT ? OFFSET ?");
            translationQuery.bind (1, correctTranslation.toStdString ());
            translationQuery.bind (2, candidatePoolSize);
            translationQuery.bind (3, offset);

            while (translationQuery.executeStep ())
            {
                std::string translation = translationQuery.getColumn (0)
                                              .getString ();
                if (!translation.empty () &&
                    seenTranslations.insert (translation).second)
                {
                    wrongTranslations.emplace_back (
                        QString::fromStdString (translation));
                }
            }
        };

        appendCandidates (randomOffset);
        if (wrongTranslations.size () < static_cast<std::size_t> (limit) &&
            randomOffset > 0)
        {
            appendCandidates (0);
        }

        if (wrongTranslations.empty ())
        {
            qCWarning (lcDb) << "No wrong translations found";
            return std::vector<QString> ();
        }

        std::shuffle (wrongTranslations.begin (), wrongTranslations.end (), gen);
        if (wrongTranslations.size () > static_cast<std::size_t> (limit))
        {
            wrongTranslations.resize (limit);
        }

        return wrongTranslations;
    });
}
