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

    return UserAuthResult::Success;
}

QString UserModel::hashPassword (const QString &password)
{
    QByteArray byteArray = password.toUtf8 ();
    QByteArray hashed =
        QCryptographicHash::hash (byteArray, QCryptographicHash::Sha256);

    return QString (hashed.toHex ());
}

void UserModel::loadUserData (const QJsonObject &userData)
{
    // Building...
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