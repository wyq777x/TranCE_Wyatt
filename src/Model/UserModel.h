#pragma once

#include "Utility/UserAuthResult.h"
#include <QCryptographicHash>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <qtmetamacros.h>
class UserModel
{
public:
    static UserModel &getInstance ()
    {
        static UserModel instance;
        return instance;
    }

    UserModel (const UserModel &) = delete;
    UserModel &operator= (const UserModel &) = delete;
    UserModel (UserModel &&) = delete;
    UserModel &operator= (UserModel &&) = delete;

    void login (const QString &username, const QString &password);
    void registerUser (const QString &username, const QString &password);
    void logout ();

    UserAuthResult Authenticate (const QString &username,
                                 const QString &password);

    bool isLoggedIn () const { return loggedIn; };
    bool isLoginExpired () const { return loginExpired; };

private:
    UserModel () = default;
    static QString hashPassword (const QString &password);
    static void loadUserData ();
    static void saveUserData ();

    bool loggedIn = false;
    bool loginExpired = false;
    inline static const QString storageFile = "users.json";
};

/*
#pragma once

#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QCryptographicHash>

class UserStorageModel {
public:
    static bool registerUser(const QString &username, const QString &password);
    static bool loginUser(const QString &username, const QString &password);

private:
    static QString hashPassword(const QString &password);
    static QJsonObject loadUserData();
    static void saveUserData(const QJsonObject &data);

    static const QString storageFile;
};




#include "UserStorageModel.h"

const QString UserStorageModel::storageFile = "users.json";

bool UserStorageModel::registerUser(const QString &username, const QString
&password) { QJsonObject users = loadUserData();

    if (users.contains(username)) {
        qWarning() << "User already exists!";
        return false;
    }

    QString hashedPassword = hashPassword(password);
    users[username] = hashedPassword;

    saveUserData(users);
    return true;
}

bool UserStorageModel::loginUser(const QString &username, const QString
&password) { QJsonObject users = loadUserData();

    if (!users.contains(username)) {
        qWarning() << "User does not exist!";
        return false;
    }

    QString storedHash = users[username].toString();
    QString inputHash = hashPassword(password);

    return storedHash == inputHash;
}

QString UserStorageModel::hashPassword(const QString &password) {
    QByteArray salt = "your_salt_value"; // 可以使用随机生成的盐值
    QByteArray hash = QCryptographicHash::hash(password.toUtf8() + salt,
QCryptographicHash::Sha256); return QString(hash.toHex());
}

QJsonObject UserStorageModel::loadUserData() {
    QFile file(storageFile);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open user storage file!";
        return QJsonObject();
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    return doc.object();
}

void UserStorageModel::saveUserData(const QJsonObject &data) {
    QFile file(storageFile);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Could not open user storage file for writing!";
        return;
    }

    QJsonDocument doc(data);
    file.write(doc.toJson());
    file.close();
}
*/