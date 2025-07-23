#include "Controller/DbManager.h"
#include "Model/DbModel.h"
#include "Utility/Result.h"

void DbManager::initDBs () { DbModel::getInstance ().initDBs (); }

AsyncTask<void> DbManager::initializeAsync (const QString &dictPath)
{
    return DbModel::getInstance ().initializeAsync (dictPath);
}

void DbManager::deleteUser (const QString &username)
{
    DbModel::getInstance ().deleteUser (username);
}
RegisterUserResult DbManager::registerUser (const QString &username,
                                            const QString &passwordHash)
{
    return DbModel::registerUser (username, passwordHash);
}

std::optional<WordEntry> DbManager::lookupWord (const QString &word,
                                                const QString &lang)
{
    return DbModel::getInstance ().lookupWord (word, lang);
}

std::vector<WordEntry> DbManager::searchWords (const QString &pattern,
                                               const QString &lang, int limit)
{
    return DbModel::getInstance ().searchWords (pattern, lang, limit);
}

ChangeResult DbManager::changeUsername (const QString &oldUsername,
                                        const QString &newUsername)
{

    return DbModel::getInstance ().changeUsername (oldUsername, newUsername);
}

ChangeResult DbManager::changePassword (const QString &username,
                                        const QString &oldPasswordHash,
                                        const QString &newPasswordHash)
{
    return DbModel::getInstance ().updateUserPassword (
        username, oldPasswordHash, newPasswordHash);
}

ChangeResult DbManager::changeEmail (const QString &username,
                                     const QString &oldEmail,
                                     const QString &newEmail)
{
    return DbModel::getInstance ().changeEmail (username, oldEmail, newEmail);
}

ChangeResult DbManager::changeAvatar (const QString &username,
                                      const QString &avatarPath)
{
    return DbModel::getInstance ().changeAvatar (username, avatarPath);
}

std::optional<QString> DbManager::getUserId (const QString &username) const
{
    return DbModel::getInstance ().getUserId (username);
}

QString DbManager::getUserEmail (const QString &username)
{
    return DbModel::getInstance ().getUserEmail (username);
}

QString DbManager::getUserAvatarPath (const QString &username)
{
    return DbModel::getInstance ().getUserAvatarPath (username);
}

std::optional<WordEntry> DbManager::getRandomWord ()
{
    return DbModel::getInstance ().getRandomWord ();
}