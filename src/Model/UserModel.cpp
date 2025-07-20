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
    ValidationResult result;

    if (userData.isEmpty ())
    {
        result.isValid = false;
        result.ErrMessages.append ("User data is empty");

        return result;
    }

    if (!userData.contains ("userAccount"))
    {
        result.isValid = false;
        result.ErrMessages.append ("User account data is missing");

        return result;
    }
    else
    {
        if (!userData["userAccount"].isObject ())
        {
            result.isValid = false;
            result.ErrMessages.append ("User account data is not an object");

            return result;
        }

        if (!userData["userAccount"].toObject ().contains ("username"))
        {
            result.isValid = false;
            result.ErrMessages.append (
                "Username is missing in user account data");

            return result;
        }

        if (!userData["userAccount"].toObject ().contains ("user_uuid"))
        {
            result.isValid = false;
            result.ErrMessages.append (
                "User UUID is missing in user account data");

            return result;
        }

        if (!userData["userAccount"].toObject ()["username"].isString ())
        {
            result.isValid = false;
            result.ErrMessages.append (
                "Username is not a string in user account data");

            return result;
        }

        if (!userData["userAccount"].toObject ()["user_uuid"].isString ())
        {
            result.isValid = false;
            result.ErrMessages.append (
                "User UUID is not a string in user account data");

            return result;
        }
    }

    if (!userData.contains ("appSettings"))
    {
        result.isValid = false;
        result.ErrMessages.append ("App settings data is missing");

        return result;
    }
    else
    {
        if (!userData["appSettings"].isObject ())
        {
            result.isValid = false;
            result.ErrMessages.append ("App settings data is not an object");

            return result;
        }

        if (!userData["appSettings"].toObject ().contains ("language"))
        {
            result.isValid = false;
            result.ErrMessages.append (
                "Language setting is missing in app settings data");

            return result;
        }

        if (!userData["appSettings"].toObject ().contains (
                "HistoryListEnabled"))
        {
            result.isValid = false;
            result.ErrMessages.append (
                "History list enabled setting is missing in app settings data");

            return result;
        }
        if (!userData["appSettings"]
                 .toObject ()["HistoryListEnabled"]
                 .isBool ())
        {
            result.isValid = false;
            result.ErrMessages.append ("History list enabled setting is not a "
                                       "boolean in app settings data");

            return result;
        }
    }

    result.isValid = true;
    result.ErrMessages.clear ();
    return result;
}

void UserModel::loadUserData (const QJsonObject &userData)
{
    // Building...
    auto validationResult = validateUserData (userData);
    if (!validationResult.isValid)
    {
        std::string errorMsg =
            "User data is invalid: " +
            validationResult.ErrMessages.join (", ").toStdString ();
        auto exception = std::runtime_error (errorMsg);
        getInstance ().logErr ("User data validation failed", exception);
        throw exception;
    }
    else
    {
    }
}

void UserModel::createUserData (const QString &filename,
                                const QString &username)
{
    try
    {
        QJsonObject userData;

        QJsonObject userAccount;
        userAccount["username"] = username;
        userAccount["user_uuid"] =
            AccountManager::getInstance ().getUserUuid (username);

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