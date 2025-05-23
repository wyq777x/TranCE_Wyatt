#include "DbModel.h"
#include "SQLiteCpp/Exception.h"
#include "Utility/Result.h"
#include <quuid.h>
#include <stdexcept>

bool DbModel::isUserDbOpen () const
{
    if (user_db)
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
    if (dict_db)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool DbModel::userExists (const QString &username) const
{
    if (!isUserDbOpen ())
    {
        return false;
    }

    try
    {
        SQLite::Statement query (
            *user_db, "SELECT COUNT(*) FROM users WHERE username = ?");
        query.bind (1, username.toStdString ());
        query.executeStep ();
        return query.getColumn (0).getInt () > 0;
    }
    catch (const SQLite::Exception &e)
    {
        logErr ("Error checking if user exists", e);
        return false;
    }
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
            *(instance.user_db),
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
        instance.logErr ("Error inserting user into database", e);
        return RegisterUserResult::DatabaseError;
    }
    catch (const std::exception &e)
    {
        instance.logErr ("Unknown error inserting user into database", e);
        return RegisterUserResult::UnknownError;
    }
    catch (...)
    {
        instance.logErr ("Unknown error inserting user into database",
                         std::runtime_error ("Unknown exception"));
        return RegisterUserResult::UnknownError;
    }
}

UserAuthResult DbModel::verifyUser (const QString &username,
                                    const QString &passwordHash) const
{
    try
    {
        return UserAuthResult::Success;
    }
    catch (...)
    {

        return UserAuthResult::UnknownError;
    }
}

void DbModel::deleteUser (const QString &username)
{
    if (!isUserDbOpen ())
    {
        return;
    }

    try
    {
        SQLite::Statement query (*user_db,
                                 "DELETE FROM users WHERE username = ?");
        query.bind (1, username.toStdString ());
        query.exec ();
    }
    catch (const SQLite::Exception &e)
    {
        logErr ("Error deleting user from database", e);
    }
}

void DbModel::updateUserPassword (const QString &username,
                                  const QString &oldPasswordHash,
                                  const QString &newPasswordHash)
{
    if (!isUserDbOpen ())
    {
        return;
    }

    try
    {
        SQLite::Statement query (*user_db,
                                 "UPDATE users SET password_hash = ? "
                                 "WHERE username = ? AND password_hash = ?");
        query.bind (1, newPasswordHash.toStdString ());
        query.bind (2, username.toStdString ());
        query.bind (3, oldPasswordHash.toStdString ());
        query.exec ();
    }
    catch (const SQLite::Exception &e)
    {
        logErr ("Error updating user password in database", e);
    }
}

std::optional<WordEntry> DbModel::lookupWord (const QString &word,
                                              const QString &lang)
{
    if (!isDictDbOpen ())
    {
        return std::nullopt;
    }

    return std::nullopt;
}

std::vector<WordEntry> DbModel::searchWords (const QString &pattern,
                                             const QString &lang, int limit)
{
    return std::vector<WordEntry> ();
}

void DbModel::addToUserVocabulary (const QString &userId, const QString &word)
{
    return;
}

void DbModel::removeFromUserVocabulary (const QString &userId,
                                        const QString &word)
{

    return;
}

void updateWordStatus (const QString &userId, const QString &word, int status)
{
    // status: -1= never learned,0=learning, 1=mastered
    return;
}

std::vector<WordEntry> getUserVocabulary (const QString &userId,
                                          int status = -1) // -1 means all
{

    return std::vector<WordEntry> ();
}