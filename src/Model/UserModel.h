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
#