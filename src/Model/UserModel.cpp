#include "UserModel.h"
#include "AccountManager.h"
#include "AppSettingModel.h"
#include "Utility/Result.h"
#include <qcontainerfwd.h>
#include <qcryptographichash.h>
#include <qdebug.h>
#include <qjsonobject.h>
#include <qlogging.h>
#include <qnamespace.h>

UserAuthResult UserModel::login (const QString &username,
                                 const QString &password)
{
    auto &instance = UserModel::getInstance ();
    if (instance.isLoggedIn ())
    {
        return UserAuthResult::UserAlreadyLoggedIn;
    }

    auto result = DbModel::getInstance ().verifyUser (
        username, AccountManager::hashPassword (password));

    if (result == UserAuthResult::Success)
    {
        instance.setLoggedIn (true);
        instance.setLoginExpired (false);

        // Load user data then

        // loadUserData (DbModel::getInstance ().getUserData (username));
    }

    return result;
}

void UserModel::logout ()
{

    auto &instance = UserModel::getInstance ();

    if (instance.isLoggedIn ())
    {
        instance.setLoggedIn (false);
        instance.setLoginExpired (true);
    }
}

ValidationResult UserModel::validateUserData (const QJsonObject &userData)
{
    ValidationResult result{true, {}};
    return result;
}
void UserModel::loadUserData (const QJsonObject &userData)
{
    // Building...
    auto validation = validateUserData (userData);
    if (!validation.isValid)
    {
        std::string errorMsg =
            "User data is invalid: " +
            validation.ErrMessages.join (", ").toStdString ();
        auto exception = std::runtime_error (errorMsg);
        getInstance ().logErr ("User data validation failed", exception);
        throw exception;
    }
}

void UserModel::saveUserData (const QString &filename)
{
    // Building...
    try
    {
        QJsonObject userData;

        QJsonObject userAccount;
        userAccount["username"] = AccountManager::getInstance ().getUsername ();
        userAccount["user_uuid"] =
            AccountManager::getInstance ().getUserUuid ();

        userData["userAccount"] = userAccount;

        QJsonObject appSettings;
        appSettings["language"] = AccountManager::getInstance ().getLanguage ();
        appSettings["HistoryListEnabled"] =
            AppSettingModel::getInstance ().isHistoryListEnabled ();
        userData["appSettings"] = appSettings;

        if (m_userProfileDir.isEmpty ())
        {
            auto exception =
                std::runtime_error ("User profile directory is not set");
            getInstance ().logErr ("User profile directory not set", exception);
            throw exception;
        }

        QDir dir (m_userProfileDir);
        if (!dir.exists ())
        {
            if (!dir.mkpath ("."))
            {
                auto exception = std::runtime_error (
                    "Failed to create user profile directory: " +
                    m_userProfileDir.toStdString ());
                getInstance ().logErr ("Directory creation failed", exception);
                throw exception;
            }
        }

        QFile file (dir.filePath (filename));

        if (!file.open (QIODevice::WriteOnly))
        {
            auto exception = std::runtime_error (
                "Failed to open file for writing: " + filename.toStdString ());
            getInstance ().logErr ("File open error", exception);
            throw exception;
        }

        QJsonDocument doc (userData);
        file.write (doc.toJson ());
        file.close ();
        qDebug () << "User data saved to" << filename;
    }

    catch (const std::exception &e)
    {
        getInstance ().logErr ("Error loading user data", e);
    }
    catch (...)
    {
        getInstance ().logErr ("Unknown error occurred while loading user data",
                               std::runtime_error ("Unknown exception"));
    }
}