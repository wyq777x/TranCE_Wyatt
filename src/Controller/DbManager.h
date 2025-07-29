#pragma once

#include "Utility/AsyncTask.h"
#include "Utility/Result.h"
#include "Utility/WordEntry.h"
#include <optional>
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

    std::optional<QString> getUserId (const QString &username) const;
    QString getUserEmail (const QString &username);
    QString getUserAvatarPath (const QString &username);

    std::optional<WordEntry> getRandomWord ();

    void addToSearchHistory (const QString &userId, const QString &word);
    std::vector<QString> getUserSearchHistory (const QString &userId);
    std::vector<WordEntry>
    getUserHistorySearchVector (const QString &user_uuid);

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