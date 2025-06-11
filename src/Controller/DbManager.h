#pragma once

#include "Utility/AsyncTask.h"
#include "Utility/Result.h"
#include "Utility/WordEntry.h"
#include <optional>
#include <vector>

class QString;

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

    std::optional<WordEntry> lookupWord (const QString &word,
                                         const QString &lang);
    std::vector<WordEntry> searchWords (const QString &pattern,
                                        const QString &lang, int limit = 10);

    ChangeResult changeUsername (const QString &oldUsername,
                                 const QString &newUsername);
    ChangeResult changePassword (const QString &username,
                                 const QString &oldPasswordHash,
                                 const QString &newPasswordHash);
    ChangeResult changeEmail (const QString &newEmail);

private:
    DbManager () = default;
    DbManager (const DbManager &) = delete;
    DbManager &operator= (const DbManager &) = delete;
    DbManager (DbManager &&) = delete;
    DbManager &operator= (DbManager &&) = delete;
};