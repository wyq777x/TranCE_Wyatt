#include "UserModel.h"
#include "AccountManager.h"
#include "AppSettingModel.h"
#include "Utility/UserAuthResult.h"
#include <qcontainerfwd.h>
#include <qcryptographichash.h>
#include <qdebug.h>
#include <qlogging.h>

UserAuthResult UserModel::login (const QString &username,
                                 const QString &password)
{
    // Building...

    auto &instance = UserModel::getInstance ();
    if (instance.loggedIn)
    {
        return UserAuthResult::UserAlreadyLoggedIn;
    }

    UserAuthResult result = Authenticate (username, password);

    if (result == UserAuthResult::Success)
    {
        instance.loggedIn = true;
        instance.loginExpired = false;
    }
    return result;
}

void UserModel::registerUser (const QString &username, const QString &password)
{
    // Building...
}

void UserModel::logout () {}

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

void UserModel::saveUserData ()
{
    // Building...
}