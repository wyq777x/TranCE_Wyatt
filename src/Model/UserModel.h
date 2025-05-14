#pragma once

#include "Utility/UserAuthResult.h"
#include <QCryptographicHash>
#include <QDebug>
#include <QFile>
#include <QJsonArray>
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
    void logout ();

    bool isLoggedIn () const { return loggedIn; };
    bool isLoginExpired () const { return loginExpired; };

    void setLoggedIn (bool loggedIn)
    {
        this->loggedIn = loggedIn;
        if (loggedIn)
            loginExpired = false;
    }
    void setLoginExpired (bool expired) { loginExpired = expired; }

    QJsonArray getUserList (const QString &storageFile) const
    {
        // Building ...
        QFile file (storageFile);
        if (!file.open (QIODevice::ReadOnly))
            return QJsonArray ();

        QJsonDocument doc = QJsonDocument::fromJson (file.readAll ());
        file.close ();
        return doc.array ();
    }

private:
    UserModel () = default;

    static void loadUserData (const QJsonObject &userData);
    static void saveUserData (const QString &filename);

    bool loggedIn = false;
    bool loginExpired = true;
    inline static const QString storageFile = "users.json";
};
#