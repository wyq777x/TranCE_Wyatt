#pragma once

#include "Utility/AsyncTask.h"
#include "Utility/Result.h"
#include "Utility/WordEntry.h"
#include <optional>
#include <utility>
#include <vector>

class DbManager
{

public:
    static DbManager &getInstance ()
    {
        static DbManager instance;
        return instance;
    }

    void initDBs ();
    AsyncTask<void> initializeAsync (const QString &dictPath = "");

    // User management functions
    RegisterUserResult registerUser (const QString &username,
                                     const QString &passwordHash);

    void deleteUser (const QString &username);
    std::optional<WordEntry> lookupWord (const QString &word,
                                         const QString &lang);
    std::vector<WordEntry> searchWords (const QString &pattern,
                                        const QString &lang, int limit = 10);

    ChangeResult changeUsername (const QString &oldUsername,
                                 const QString &newUsername);
    ChangeResult changePassword (const QString &username,
                                 const QString &oldPasswordHash,
                                 const QString &newPasswordHash);
    ChangeResult changeEmail (const QString &username, const QString &oldEmail,
                              const QString &newEmail);

    ChangeResult changeAvatar (const QString &username,
                               const QString &avatarPath);

    ChangeResult updateReciteProgress (int current, int total,
                                       const QString &userId);

    ChangeResult updateWordStatus (const QString &userId, const QString &word,
                                   int status);

    std::optional<QString> getUserId (const QString &username) const;
    QString getUserEmail (const QString &username);
    QString getUserAvatarPath (const QString &username);

    std::optional<WordEntry> getRandomWord ();

    void addToUserFavorites (const QString &userId, const QString &word);
    void removeFromUserFavorites (const QString &userId, const QString &word);
    bool isWordFavorited (const QString &userId, const QString &word);

    bool existsInMastered (const QString &userId, const QString &word);

    void addToSearchHistory (const QString &userId, const QString &word);
    void removeFromSearchHistory (const QString &userId, const QString &word);

    void addToReciteHistory (const QString &userId, const QString &word);
    void removeFromReciteHistory (const QString &userId, const QString &word);

    std::vector<QString> getUserSearchHistory (const QString &userId);
    std::vector<WordEntry>
    getUserHistorySearchVector (const QString &user_uuid);
    std::vector<QString> getUserReciteHistory (const QString &userId);

    std::pair<int, int> getProgress (const QString &userId) const;

    std::vector<QString>
    getRandomWrongTranslations (const QString &correctTranslation, int limit);

private:
    DbManager () = default;
    DbManager (const DbManager &) = delete;
    DbManager &operator= (const DbManager &) = delete;
    DbManager (DbManager &&) = delete;
    DbManager &operator= (DbManager &&) = delete;

    mutable std::string m_lastError;

    template <typename ExceptionT>
    void logErr (const std::string &errMsg, const ExceptionT &e)
    {
        qCritical () << "[DbManager]" << QString::fromStdString (errMsg)
                     << "Exception:" << e.what ();
        m_lastError = errMsg + " Exception:" + std::string (e.what ());
    }
};