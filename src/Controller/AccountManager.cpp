#include "AccountManager.h"
#include "UserModel.h"
#include "Utility/UserAuthResult.h"
#include <qcontainerfwd.h>

void AccountManager::logout ()
{

    auto &instance = UserModel::getInstance ();
    instance.logout ();
    instance.setLoggedIn (false);
    instance.setLoginExpired (true);
    // clear the username and password of View
    // ...
}

void AccountManager::login (const QString &username, const QString &password)
{
    auto &instance = UserModel::getInstance ();

    auto result = instance.login (username, password);

    return;
}

void AccountManager::registerUser (const QString &username,
                                   const QString &password)
{
    auto &instance = UserModel::getInstance ();

    return;
}

QString AccountManager::hashPassword (const QString &password)
{
    QByteArray byteArray = password.toUtf8 ();
    QByteArray hashed =
        QCryptographicHash::hash (byteArray, QCryptographicHash::Sha256);

    return QString (hashed.toHex ());
}

QString AccountManager::getUsername () const { return username; }

QString AccountManager::getHashedPassword () const { return password_Hash; }

QString AccountManager::getLanguage () const
{
    return AppSettingModel::getInstance ().getLanguage ();
}