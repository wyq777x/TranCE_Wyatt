#pragma once

#include "Utility/Constants.h"
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

class SettingManager;

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

    static ValidationResult validateUserData (const QJsonObject &userData);

    static UserDataResult loadUserData (const QString &userProfilePath);

    UserDataResult createUserData (const QString &filename,
                                   const QString &username);

    ChangeResult
    changeHistorySearchListEnabled_Json (bool enabled,
                                         const QString &userProfile);

    ChangeResult changeLanguage_Json (const QString &lang,
                                      const QString &userProfile);

    QString getUserProfileDir ()
    {
        return QCoreApplication::applicationDirPath () +
               Constants::Paths::USER_PROFILES_DIR;
    }

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