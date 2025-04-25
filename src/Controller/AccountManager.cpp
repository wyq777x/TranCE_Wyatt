#include "AccountManager.h"
#include "UserModel.h"
#include "Utility/UserAuthResult.h"
#include <qcontainerfwd.h>

void AccountManager::logout ()
{

    if (userModel)
        userModel->logout ();
    username.clear ();
    password_Hash.clear ();
}

void AccountManager::login (const QString &username, const QString &password)
{
    if (userModel)
    {
        auto result = userModel->login (username, password);
        if (result == UserAuthResult::Success)
        {
            this->username = username;
            this->password_Hash = hashPassword (password);
        }
    }
}

void AccountManager::registerUser (const QString &username,
                                   const QString &password)
{
    if (userModel)
        userModel->registerUser (username, password);
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