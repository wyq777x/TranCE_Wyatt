#pragma once

#include "Utility/AsyncTask.h"
#include "Utility/Result.h"
#include "Utility/WordEntry.h"
#include <QString>
#include <QStringList>
#include <cstddef>
#include <exception>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace SQLite
{
class Database;
}

class DbModel
{
public:
    DbModel (const DbModel &) = delete;
    DbModel &operator= (const DbModel &) = delete;

    DbModel (DbModel &&) noexcept;
    DbModel &operator= (DbModel &&) noexcept;
    ~DbModel ();

    static DbModel &getInstance ()
    {
        static DbModel instance;
        return instance;
    }

    static QString getDbDir ();
    static QString getAvatarDir ();

    void initDBs ();

    // Asynchronous init method
    AsyncTask<void> initializeAsync (const QString &dictPath = "");
    // Database connection and management

    bool isUserDbOpen () const;
    bool isDictDbOpen () const;
    bool isDictionaryImportInProgress () const;
    bool isDictionaryReady () const;

    // User

    bool userExists (const QString &username) const;

    bool userIdExists (const QString &userId) const;

    static RegisterUserResult registerUser (const QString &username,
                                            const QString &passwordHash);

    UserAuthResult verifyUser (const QString &username,
                               const QString &password) const;

    void deleteUser (const QString &username);

    ChangeResult updateUserPassword (const QString &username,
                                     const QString &oldPassword,
                                     const QString &newPassword);

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
    QString getUserPasswordHash (const QString &username) const;
    QString getUserEmail (const QString &username) const;
    QString getUserAvatarPath (const QString &username) const;

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
    void clearWordLookupCache ();
    std::size_t getWordLookupCacheSizeBytes () const;

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

    std::string getLastError () const;

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
    struct Impl;

    explicit DbModel ();

    static std::string buildWordLookupCacheKey (const QString &word,
                                                const QString &srcLang);
    void refreshDictImportRequirement ();
    static void configureDictDatabasePragmas (SQLite::Database &database);
    static int countDictionaryWords (SQLite::Database &database);
    static void
    insertWordBatchSyncInternal (SQLite::Database &database,
                                 const std::vector<WordEntry> &batch);
    static void importFromFileSyncInternal (
        SQLite::Database &database, const QString &filePath,
        std::function<void (int, int)> progressCallback = nullptr);

    void initUserTable ();
    void initDictTable ();

    void logErr (const std::string &errMsg, const std::exception &e) const;

    static QString resolveResourcePath (const QString &relativePath);

    static WordEntry parseCSVLineToWordEntry (const QString &csvLine);
    static QStringList parseCSVLine (const QString &line);

    std::unique_ptr<Impl> d;
};
