#include "DbManager.h"
#include "Model/DbModel.h"

void DbManager::initDBs () { DbModel::getInstance ().initDBs (); }

AsyncTask<void> DbManager::initializeAsync (const QString &dictPath)
{
    return DbModel::getInstance ().initializeAsync (dictPath);
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

QString DbManager::getUserEmail (const QString &username)
{
    return DbModel::getInstance ().getUserEmail (username);
}