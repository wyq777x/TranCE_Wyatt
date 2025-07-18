#include "UserModel.h"
#include "AccountManager.h"
#include "AppSettingModel.h"
#include "Utility/Result.h"
#include <qcontainerfwd.h>
#include <qcryptographichash.h>
#include <qdebug.h>
#include <qjsonobject.h>
#include <qlogging.h>
#include <qnamespace.h>

UserAuthResult UserModel::login (const QString &username,
                                 const QString &password)
{
    auto &instance = UserModel::getInstance ();
    if (instance.isLoggedIn ())
    {
        return UserAuthResult::UserAlreadyLoggedIn;
    }

    auto result = DbModel::getInstance ().verifyUser (
        username, AccountManager::hashPassword (password));

    if (result == UserAuthResult::Success)
    {
        instance.setLoggedIn (true);
        instance.setLoginExpired (false);

        // Load user data then

        // loadUserData (DbModel::getInstance ().getUserData (username));
    }

    return result;
}

void UserModel::logout ()
{

    auto &instance = UserModel::getInstance ();

    if (instance.isLoggedIn ())
    {
        instance.setLoggedIn (false);
        instance.setLoginExpired (true);
    }
}

ValidationResult UserModel::validateUserData (const QJsonObject &userData)
{
    ValidationResult result{true, {}};
    return result;
}
void UserModel::loadUserData (const QJsonObject &userData)
{
    // Building...
    auto validation = validateUserData (userData);
    if (!validation.isValid)
    {
        std::string errorMsg =
            "User data is invalid: " +
            validation.ErrMessages.join (", ").toStdString ();
        auto exception = std::runtime_error (errorMsg);
        getInstance ().logErr ("User data validation failed", exception);
        throw exception;
    }
}

void UserModel::saveUserData (const QString &filename)
{
    // Building...
    try
    {
        QJsonObject userData;

        QJsonObject userAccount;
        userAccount["username"] = AccountManager::getInstance ().getUsername ();
        userAccount["user_uuid"] =
            AccountManager::getInstance ().getUserUuid ();

        userData["userAccount"] = userAccount;

        QJsonObject appConfig;
        appConfig["language"] = AccountManager::getInstance ().getLanguage ();
        appConfig["HistoryListEnabled"] =
            AppSettingModel::getInstance ().isHistoryListEnabled ();
        userData["appConfig"] = appConfig;
    }

    catch (const std::exception &e)
    {
        getInstance ().logErr ("Error loading user data", e);
    }
    catch (...)
    {
        getInstance ().logErr ("Unknown error occurred while loading user data",
                               std::runtime_error ("Unknown exception"));
    }
}