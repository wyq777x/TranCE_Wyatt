```cpp
#pragma once

#include <QCryptographicHash>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QDebug>

class UserStorage {
public:
    static bool registerUser(const QString &username, const QString &password);
    static bool loginUser(const QString &username, const QString &password);

private:
    static QString hashPassword(const QString &password);
    static QJsonObject loadUserData();
    static void saveUserData(const QJsonObject &data);

    static const QString storageFile;
};

class UserModel
{
public:
    static UserModel &getInstance()
    {
        static UserModel instance;
        return instance;
    }

    UserModel(const UserModel &) = delete;
    UserModel &operator=(const UserModel &) = delete;
    UserModel(UserModel &&) = delete;
    UserModel &operator=(UserModel &&) = delete;

    bool isLoggedIn() const { return loggedIn; }
    bool isLoginExpired() const { return loginExpired; }

    bool login(const QString &username, const QString &password)
    {
        if (UserStorage::loginUser(username, password)) {
            loggedIn = true;
            loginExpired = false; /
            return true;
        }
        return false;
    }

    bool registerUser(const QString &username, const QString &password)
    {
        return UserStorage::registerUser(username, password);
    }

private:
    UserModel() = default;

    bool loggedIn = false;
    bool loginExpired = false;
};

// filepath: /home/wyq777/Desktop/Code/TranCE_Wyatt/src/Model/UserModel.cpp
#include "UserModel.h"

const QString UserStorage::storageFile = "users.json";

bool UserStorage::registerUser(const QString &username, const QString &password)
{
    QJsonObject users = loadUserData();

    if (users.contains(username)) {
        qWarning() << "User already exists!";
        return false;
    }

    QString hashedPassword = hashPassword(password);
    users[username] = hashedPassword;

    saveUserData(users);
    return true;
}

bool UserStorage::loginUser(const QString &username, const QString &password)
{
    QJsonObject users = loadUserData();

    if (!users.contains(username)) {
        qWarning() << "User does not exist!";
        return false;
    }

    QString storedHash = users[username].toString();
    QString inputHash = hashPassword(password);

    return storedHash == inputHash;
}

QString UserStorage::hashPassword(const QString &password)
{
    QByteArray salt = "your_salt_value"; // 可以使用随机生成的盐值
    QByteArray hash = QCryptographicHash::hash(password.toUtf8() + salt, QCryptographicHash::Sha256);
    return QString(hash.toHex());
}

QJsonObject UserStorage::loadUserData()
{
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

void UserStorage::saveUserData(const QJsonObject &data)
{
    QFile file(storageFile);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Could not open user storage file for writing!";
        return;
    }

    QJsonDocument doc(data);
    file.write(doc.toJson());
    file.close();
}
```