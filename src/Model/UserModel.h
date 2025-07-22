#pragma once

#include "Controller/Setting.h"
#include "Utility/Result.h"
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <qjsonobject.h>
#include <qtmetamacros.h>

class UserModel : public QObject
{
    Q_OBJECT
public:
    static UserModel &getInstance ()
    {
        static UserModel instance (QCoreApplication::applicationDirPath () +
                                   "/Utility/UserProfiles");
        return instance;
    }

    UserModel () = delete;
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
    static ValidationResult validateUserData (const QJsonObject &userData);

    static UserDataResult loadUserData (const QString &userProfilePath);

    UserDataResult createUserData (const QString &filename,
                                   const QString &username);

private:
    explicit UserModel (const QString &userProfilePath)
        : m_userProfileDir (userProfilePath)
    {
        QDir dir (m_userProfileDir);
        if (!dir.exists ())
        {
            dir.mkpath (".");
        }
    }

    QString m_userProfileDir;

    bool loggedIn = false;
    bool loginExpired = true;

    mutable std::string m_lastError;

    template <typename ExceptionT>
    void logErr (const std::string &errMsg, const ExceptionT &e)
    {
        qCritical () << "[UserModel]" << QString::fromStdString (errMsg)
                     << "Exception:" << e.what ();
        m_lastError = errMsg + " Exception:" + std::string (e.what ());
    }
};