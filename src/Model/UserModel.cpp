#include "Model/UserModel.h"
#include "Controller/AccountManager.h"
#include "Controller/SettingManager.h"
#include "Model/AppSettingModel.h"
#include "Model/DbModel.h"
#include "Utility/Constants.h"
#include "Utility/Result.h"
#include <type_traits>

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

UserDataResult UserModel::loadUserData (const QString &userProfile)
{
    try
    {
        // Building...
        if (userProfile.isEmpty ())
        {
            getInstance ().logErr (
                "User profile path not set",
                std::runtime_error ("User profile path is not set or empty"));
            return UserDataResult::EmptyPath;
        }

        QDir userProfileDir = QDir (getInstance ().getUserProfileDir ());
        if (!userProfileDir.exists ())
        {
            getInstance ().logErr (
                "User profile directory not found",
                std::runtime_error ("User profile directory does not exist: " +
                                    userProfileDir.path ().toStdString ()));
            return UserDataResult::DirectoryNotFound;
        }

        QString userProfilePath = userProfileDir.filePath (userProfile);

        QFile userProfileFile (userProfilePath);
        if (!userProfileFile.exists ())
        {
            getInstance ().logErr (
                "User profile file not found",
                std::runtime_error ("User profile file does not exist: " +
                                    userProfilePath.toStdString ()));
            return UserDataResult::FileNotFound;
        }

        if (!userProfileFile.open (QIODevice::ReadOnly))
        {
            getInstance ().logErr (
                "User profile file open error",
                std::runtime_error ("Failed to open user profile file: " +
                                    userProfile.toStdString ()));
            return UserDataResult::FileOpenError;
        }

        QByteArray fileData = userProfileFile.readAll ();
        userProfileFile.close ();

        if (fileData.isEmpty ())
        {
            getInstance ().logErr (
                "User profile file read error",
                std::runtime_error (
                    "Failed to read user profile file or file is empty: " +
                    userProfile.toStdString ()));
            return UserDataResult::FileReadError;
        }

        QJsonDocument userDataDoc = QJsonDocument::fromJson (fileData);
        if (userDataDoc.isNull () || !userDataDoc.isObject ())
        {
            getInstance ().logErr (
                "User profile file parse error",
                std::runtime_error (
                    "Failed to parse JSON from user profile file: " +
                    userProfile.toStdString ()));
            return UserDataResult::InvalidData;
        }

        QJsonObject userData = userDataDoc.object ();

        auto validationResult = validateUserData (userData);
        if (!validationResult.isValid)
        {
            std::string errorMsg =
                "User data is invalid: " +
                validationResult.ErrMessages.join (", ").toStdString ();
            getInstance ().logErr ("User data validation failed",
                                   std::runtime_error (errorMsg));
            return UserDataResult::InvalidData;
        }

        // Load user data
        QJsonObject appSettings = userData.value ("appSettings").toObject ();
        QString language = appSettings.value ("language").toString ();
        bool historyListEnabled =
            appSettings.value ("HistoryListEnabled").toBool ();

        if (language.isEmpty ())
        {
            getInstance ().logErr (
                "App settings data is incomplete",
                std::runtime_error (
                    "Language setting is missing in app settings data"));
            return UserDataResult::InvalidData;
        }

        // Set settings

        auto setHistoryResult =
            SettingManager::getInstance ().setHistorySearchListEnabled (
                historyListEnabled);
        auto setLangResult =
            SettingManager::getInstance ().setLanguage (language);

        if (setHistoryResult != ChangeResult::Success ||
            setLangResult != ChangeResult::Success)
        {
            getInstance ().logErr (
                "Failed to set app settings",
                std::runtime_error (
                    "Failed to set app settings from user profile data"));
            return UserDataResult::SettingError;
        }

        // update UI
        return UserDataResult::Success;
    }
    catch (const std::exception &e)
    {
        getInstance ().logErr ("Error loading user data", e);
        return UserDataResult::UnknownError;
    }
    catch (...)
    {
        getInstance ().logErr ("Unknown error occurred while loading user data",
                               std::runtime_error ("Unknown exception"));
        return UserDataResult::UnknownError;
    }
}

UserDataResult UserModel::createUserData (const QString &filename,
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
            getInstance ().logErr (
                "User profile directory not set",
                std::runtime_error ("User profile directory is not set"));
            return UserDataResult::EmptyPath;
        }

        QDir dir (m_userProfileDir);
        if (!dir.exists ())
        {
            if (!dir.mkpath ("."))
            {
                getInstance ().logErr (
                    "Directory creation failed",
                    std::runtime_error (
                        "Failed to create user profile directory: " +
                        m_userProfileDir.toStdString ()));
                return UserDataResult::DirectoryCreateError;
            }
        }

        QFile file (dir.filePath (filename));

        if (!file.open (QIODevice::WriteOnly))
        {
            getInstance ().logErr (
                "File open error",
                std::runtime_error ("Failed to open file for writing: " +
                                    filename.toStdString ()));
            return UserDataResult::FileOpenError;
        }

        QJsonDocument doc (userData);
        if (file.write (doc.toJson ()) == -1)
        {
            file.close ();
            getInstance ().logErr (
                "File write error",
                std::runtime_error ("Failed to write data to file: " +
                                    filename.toStdString ()));
            return UserDataResult::FileWriteError;
        }
        file.close ();
        qDebug () << "User data saved to" << filename;
        return UserDataResult::Success;
    }
    catch (const std::exception &e)
    {
        getInstance ().logErr ("Error creating user data", e);
        return UserDataResult::UnknownError;
    }
    catch (...)
    {
        getInstance ().logErr (
            "Unknown error occurred while creating user data",
            std::runtime_error ("Unknown exception"));
        return UserDataResult::UnknownError;
    }
}

template <typename FieldT>
ChangeResult UserModel::changeUserProfileField (const QString &field,
                                                const FieldT &value,
                                                const QString &userProfile)
{
    try
    {
        if (userProfile.isEmpty ())
        {
            logErr ("User profile file path is empty",
                    std::runtime_error ("User profile file path is empty"));
            return ChangeResult::NullValue;
        }

        QDir userProfileDir = QDir (getUserProfileDir ());
        if (!userProfileDir.exists ())
        {
            logErr ("User profile directory does not exist",
                    std::runtime_error ("User profile directory does not "
                                        "exist: " +
                                        userProfileDir.path ().toStdString ()));
            return ChangeResult::DirectoryNotFound;
        }

        QString userProfilePath = userProfileDir.filePath (userProfile);

        QFile userProfileFile (userProfilePath);
        if (!userProfileFile.exists ())
        {
            logErr ("User profile file does not exist",
                    std::runtime_error ("User profile file does not exist: " +
                                        userProfilePath.toStdString ()));
            return ChangeResult::FileNotFound;
        }

        if (!userProfileFile.open (QIODevice::ReadOnly))
        {
            logErr ("User profile file open error",
                    std::runtime_error ("Failed to open user profile file: " +
                                        userProfilePath.toStdString ()));
            return ChangeResult::FileOpenError;
        }

        QByteArray fileData = userProfileFile.readAll ();
        userProfileFile.close ();

        if (fileData.isEmpty ())
        {
            getInstance ().logErr (
                "User profile file read error",
                std::runtime_error (
                    "Failed to read user profile file or file is empty: " +
                    userProfile.toStdString ()));
            return ChangeResult::NullValue;
        }

        QJsonDocument userDataDoc = QJsonDocument::fromJson (fileData);
        if (userDataDoc.isNull () || !userDataDoc.isObject ())
        {
            logErr (
                "User profile file parse error",
                std::runtime_error ("Failed to parse JSON from user profile "
                                    "file: " +
                                    userProfilePath.toStdString ()));
            return ChangeResult::InvalidInput;
        }

        QJsonObject userData = userDataDoc.object ();
        QJsonObject appSettings = userData.value ("appSettings").toObject ();

        QJsonValue jsonValue;
        if constexpr (std::is_same_v<FieldT, bool>)
        {
            jsonValue = QJsonValue (value);
        }
        else if constexpr (std::is_same_v<FieldT, QString>)
        {
            jsonValue = QJsonValue (value);
        }
        else
        {
            jsonValue = QJsonValue::fromVariant (QVariant::fromValue (value));
        }

        appSettings[field] = jsonValue;
        userData["appSettings"] = appSettings;

        QJsonDocument updatedDoc (userData);
        if (!userProfileFile.open (QIODevice::WriteOnly | QIODevice::Truncate))
        {
            logErr ("User profile file write error",
                    std::runtime_error ("Failed to open user profile file for "
                                        "writing: " +
                                        userProfilePath.toStdString ()));
            return ChangeResult::FileOpenError;
        }

        if (userProfileFile.write (updatedDoc.toJson ()) == -1)
        {
            userProfileFile.close ();
            logErr ("User profile file write error",
                    std::runtime_error (
                        "Failed to write data to user profile file: " +
                        userProfilePath.toStdString ()));
            return ChangeResult::FileWriteError;
        }

        userProfileFile.close ();
        return ChangeResult::Success;
    }
    catch (const std::exception &e)
    {
        logErr ("Error changing user profile field: " + field.toStdString (),
                e);
        return ChangeResult::UnknownError;
    }
    catch (...)
    {
        logErr ("Unknown error occurred while changing user profile field: " +
                    field.toStdString (),
                std::runtime_error ("Unknown exception"));
        return ChangeResult::UnknownError;
    }
}

ChangeResult
UserModel::changeHistorySearchListEnabled_Json (bool enabled,
                                                const QString &userProfile)
{
    return changeUserProfileField (
        Constants::JsonFields::appSettings::HISTORY_LIST_ENABLED, enabled,
        userProfile);
}

ChangeResult UserModel::changeLanguage_Json (const QString &lang,
                                             const QString &userProfile)
{
    return changeUserProfileField (Constants::JsonFields::appSettings::LANGUAGE,
                                   lang, userProfile);
}
