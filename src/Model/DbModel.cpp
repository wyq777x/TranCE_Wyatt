#include "DbModel.h"
#include "SQLiteCpp/Exception.h"
#include "Utility/Result.h"
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

bool DbModel::userExists (const QString &username) const { return false; }

RegisterUserResult DbModel::registerUser (const QString &username,
                                          const QString &passwordHash)
{
    if (!isUserDbOpen ())
    {
        return RegisterUserResult::DatabaseError;
    }
    if (userExists (username))
    {
        return RegisterUserResult::UserAlreadyExists;
    }
    try
    {
        return RegisterUserResult::Success;
    }
    catch (const SQLite::Exception &e)
    {
        logErr ("Error inserting user into database", e);
        return RegisterUserResult::DatabaseError;
    }
    catch (const std::exception &e)
    {
        logErr ("Unknown error inserting user into database", e);
        return RegisterUserResult::UnknownError;
    }
    catch (...)
    {
        logErr ("Unknown error inserting user into database",
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
    }
}
/*

UserAuthResult UserModel::Authenticate (const QString &username,
                                        const QString &password)
{
    QJsonArray userList = getUserList ("users.json");

    for (const QJsonValue &userVal : userList)
    {
        QJsonObject userObj = userVal.toObject ();
        QString storedUsername = userObj["username"].toString ();
        QString storedPasswordHash = userObj["password"].toString ();

        if (username == storedUsername)
        {
            QString inputPasswordHash = AccountManager::hashPassword (password);
            if (inputPasswordHash == storedPasswordHash)
                return UserAuthResult::Success;
            else
                return UserAuthResult::IncorrectPassword;
        }
    }
    return UserAuthResult::UserNotFound;
}
*/