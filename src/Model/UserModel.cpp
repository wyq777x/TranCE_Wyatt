#include "UserModel.h"
#include "AccountManager.h"
#include "AppSettingModel.h"
#include "Utility/UserAuthResult.h"
#include <qcontainerfwd.h>
#include <qcryptographichash.h>
#include <qdebug.h>
#include <qjsonobject.h>
#include <qlogging.h>
#include <qnamespace.h>

UserAuthResult UserModel::login (const QString &username,
                                 const QString &password)
{
    // Building...

    auto &instance = UserModel::getInstance ();
    if (instance.isLoggedIn ())
    {
        return UserAuthResult::UserAlreadyLoggedIn;
    }

    UserAuthResult result = Authenticate (username, password);

    if (result == UserAuthResult::Success)
    {
        instance.setLoggedIn (true);
        instance.setLoginExpired (false);
    }
    return result;
}

void UserModel::registerUser (const QString &username, const QString &password)
{
    // Building...
    auto &instance = UserModel::getInstance ();
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

void UserModel::loadUserData (const QJsonObject &userData)
{
    // Building...
    if (userData.isEmpty ())
        throw std::runtime_error ("User data is empty");

    if (!userData.contains ("userAccount") ||
        !userData["userAccount"].isObject ())
        throw std::runtime_error ("User account is invalid");
    if (!userData.contains ("userProfile") ||
        !userData["userProfile"].isObject ())
        throw std::runtime_error ("User profile is invalid");

    if (!userData.contains ("appConfig") || !userData["appConfig"].isObject ())
        throw std::runtime_error ("App config is invalid");

    if (!userData.contains ("userStatus") ||
        !userData["userStatus"].isObject ())
        throw std::runtime_error ("User status is invalid");

    if (!userData.contains ("userStatistics") ||
        !userData["userStatistics"].isObject ())
        throw std::runtime_error ("User statistics is invalid");

    if (!userData["userAccount"].toObject ().contains ("username") ||
        !userData["userAccount"].toObject ()["username"].isString ())
        throw std::runtime_error ("Username is invalid");

    if (!userData["userAccount"].toObject ().contains ("password") ||
        !userData["userAccount"].toObject ()["password"].isString ())
        throw std::runtime_error ("Password is invalid");

    if (!userData["userProfile"].toObject ().contains ("avatar") ||
        !userData["userProfile"].toObject ()["avatar"].isString ())
        throw std::runtime_error ("Avatar is invalid");

    if (!userData["userProfile"].toObject ().contains ("email") ||
        !userData["userProfile"].toObject ()["email"].isString ())
        throw std::runtime_error ("Email is invalid");

    if (!userData["appConfig"].toObject ().contains ("language") ||
        !userData["appConfig"].toObject ()["language"].isString ())
        throw std::runtime_error ("Language is invalid");

    if (!userData["userStatus"].toObject ().contains ("lastLogin") ||
        !userData["userStatus"].toObject ()["lastLogin"].isString ())
        throw std::runtime_error ("Last login is invalid");

    if (!userData["userStatus"].toObject ().contains ("isLoggedIn") ||
        !userData["userStatus"].toObject ()["isLoggedIn"].isBool ())
        throw std::runtime_error ("Is logged in is invalid");

    if (!userData["userStatus"].toObject ().contains ("isLoginExpired") ||
        !userData["userStatus"].toObject ()["isLoginExpired"].isBool ())
        throw std::runtime_error ("Is login expired is invalid");

    if (!userData["userStatistics"].toObject ().contains ("masteredCount"))
        throw std::runtime_error ("Mastered count is invalid");

    if (!userData["userStatistics"].toObject ().contains ("learningCount"))
        throw std::runtime_error ("Learning count is invalid");

    try
    {
        QJsonObject userAccount = userData["userAccount"].toObject ();
        QJsonObject userProfile = userData["userProfile"].toObject ();
        QJsonObject appConfig = userData["appConfig"].toObject ();
        QJsonObject userStatus = userData["userStatus"].toObject ();
        QJsonObject userStatistics = userData["userStatistics"].toObject ();
    }
    catch (const std::exception &e)
    {
        qDebug () << "Error loading user data:" << e.what ();
        throw;
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
        userAccount["password"] =
            AccountManager::getInstance ().getHashedPassword ();

        userData["userAccount"] = userAccount;

        QJsonObject userProfile;
        userProfile["avatar"] = "default_avatar.png";
        userProfile["email"] = "*@example.com";

        userData["userProfile"] = userProfile;

        QJsonObject appConfig;
        appConfig["language"] = AccountManager::getInstance ().getLanguage ();

        userData["appConfig"] = appConfig;

        QJsonObject userStatus;
        userStatus["lastLogin"] =
            QDateTime::currentDateTime ().toString (Qt::ISODateWithMs);
        userStatus["isLoggedIn"] = AccountManager::getInstance ().isLoggedIn ();
        userStatus["isLoginExpired"] =
            AccountManager::getInstance ().isLoginExpired ();

        userData["userStatus"] = userStatus;

        QJsonObject userStatistics;

        userStatistics["masteredCount"] = 0;
        userStatistics["learningCount"] = 0;

        userData["userStatistics"] = userStatistics;
    }
    catch (const std::exception &e)
    {
        qDebug () << "Error loading user data:" << e.what ();
    }
}