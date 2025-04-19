#include "UserModel.h"
#include "AccountManager.h"
#include "AppSettingModel.h"
#include "Utility/UserAuthResult.h"
#include <qcontainerfwd.h>
#include <qcryptographichash.h>

void UserModel::login (const QString &username, const QString &password) {}

void UserModel::registerUser (const QString &username, const QString &password)
{
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

void UserModel::loadUserData () {}

void UserModel::saveUserData () {}