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
    // Building...

    auto &instance = UserModel::getInstance ();
    if (instance.isLoggedIn ())
    {
        return UserAuthResult::UserAlreadyLoggedIn;
    }

    return UserAuthResult::Success;
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
    if (userData.isEmpty ())
        result.ErrMessages << "User data is empty";
    if (!userData.contains ("userAccount") ||
        !userData["userAccount"].isObject ())
        result.ErrMessages << "User account is invalid";
    if (!userData.contains ("userProfile") ||
        !userData["userProfile"].isObject ())
        result.ErrMessages << "User profile is invalid";

    if (!userData.contains ("appConfig") || !userData["appConfig"].isObject ())
        result.ErrMessages << "App config is invalid";

    if (!userData.contains ("userStatus") ||
        !userData["userStatus"].isObject ())
        result.ErrMessages << "User status is invalid";

    if (!userData.contains ("userStatistics") ||
        !userData["userStatistics"].isObject ())
        result.ErrMessages << "User statistics is invalid";

    if (!userData["userAccount"].toObject ().contains ("username") ||
        !userData["userAccount"].toObject ()["username"].isString ())
        result.ErrMessages << "Username is invalid";

    if (!userData["userProfile"].toObject ().contains ("avatar") ||
        !userData["userProfile"].toObject ()["avatar"].isString ())
        result.ErrMessages << "Avatar is invalid";

    if (!userData["userProfile"].toObject ().contains ("email") ||
        !userData["userProfile"].toObject ()["email"].isString ())
        result.ErrMessages << "Email is invalid";

    if (!userData["appConfig"].toObject ().contains ("language") ||
        !userData["appConfig"].toObject ()["language"].isString ())
        result.ErrMessages << "Language is invalid";

    if (!userData["userStatus"].toObject ().contains ("lastLogin") ||
        !userData["userStatus"].toObject ()["lastLogin"].isString ())
        result.ErrMessages << "Last login is invalid";

    if (!userData["userStatus"].toObject ().contains ("isLoggedIn") ||
        !userData["userStatus"].toObject ()["isLoggedIn"].isBool ())
        result.ErrMessages << "Is logged in is invalid";

    if (!userData["userStatus"].toObject ().contains ("isLoginExpired") ||
        !userData["userStatus"].toObject ()["isLoginExpired"].isBool ())
        result.ErrMessages << "Is login expired is invalid";

    if (!userData["userStatistics"].toObject ().contains ("masteredCount"))
        result.ErrMessages << "Mastered count is invalid";

    if (!userData["userStatistics"].toObject ().contains ("learningCount"))
        result.ErrMessages << "Learning count is invalid";

    QJsonObject userAccount = userData["userAccount"].toObject ();
    QJsonObject userProfile = userData["userProfile"].toObject ();
    QJsonObject appConfig = userData["appConfig"].toObject ();
    QJsonObject userStatus = userData["userStatus"].toObject ();
    QJsonObject userStatistics = userData["userStatistics"].toObject ();

    result.isValid = result.ErrMessages.isEmpty ();
    return result;
}
void UserModel::loadUserData (const QJsonObject &userData)
{
    // Building...
    auto validation = validateUserData (userData);
    if (!validation.isValid)
    {
        qCritical () << "User data is invalid";
        for (const QString &err : validation.ErrMessages)
        {
            qCritical () << err;
        }
        throw std::runtime_error (
            "User data is invalid: " +
            validation.ErrMessages.join (", ").toStdString ());
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
        userAccount["userkey"] = 1;

        userData["userAccount"] = userAccount;

        QJsonObject userProfile;
        userProfile["avatar"] = ":/res/image/DefaultUser.png";
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