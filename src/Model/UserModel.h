#pragma once

#include "Utility/UserAuthResult.h"
#include <QCryptographicHash>
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <qjsonobject.h>
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

    UserAuthResult login (const QString &username, const QString &password);
    void registerUser (const QString &username, const QString &password);
    void logout ();

    UserAuthResult Authenticate (const QString &username,
                                 const QString &password);

    bool isLoggedIn () const { return loggedIn; };
    bool isLoginExpired () const { return loginExpired; };

    void setLoggedIn (bool loggedIn)
    {
        this->loggedIn = loggedIn;
        if (loggedIn)
            loginExpired = false;
    }
    void setLoginExpired (bool expired) { loginExpired = expired; }

private:
    UserModel () = default;
    static QString hashPassword (const QString &password);
    static void loadUserData (const QJsonObject &userData);
    static void saveUserData ();

    bool loggedIn = false;
    bool loginExpired = true;
    inline static const QString storageFile = "users.json";
};
#