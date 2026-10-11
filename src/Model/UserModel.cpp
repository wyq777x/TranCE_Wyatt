#include "Model/UserModel.h"
#include "Controller/SettingManager.h"
#include "Model/AppSettingModel.h"
#include "Model/DbModel.h"
#include "Utility/AspectGuard.h"
#include "Utility/Constants.h"
#include "Utility/Result.h"
#include <type_traits>

using trance::aspect::guarded;

UserAuthResult UserModel::login (const QString &username,
                                 const QString &password)
{
    auto &instance = UserModel::getInstance ();
    if (instance.isLoggedIn ())
    {
        return UserAuthResult::UserAlreadyLoggedIn;
    }

    auto result = DbModel::getInstance ().verifyUser (username, password);

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
    return guarded (lcUser, "loading user data",
                    UserDataResult::UnknownError,
                    [&]() -> UserDataResult
    {
        // Building...
        if (userProfile.isEmpty ())
        {
            qCWarning (lcUser) << "User profile path is not set or empty";
            return UserDataResult::EmptyPath;
        }

        QDir userProfileDir = QDir (getInstance ().getUserProfileDir ());
        if (!userProfileDir.exists ())
        {
            qCWarning (lcUser) << "User profile directory does not exist:"
                               << userProfileDir.path ();
            return UserDataResult::DirectoryNotFound;
        }

        QString userProfilePath = userProfileDir.filePath (userProfile);

        QFile userProfileFile (userProfilePath);
        if (!userProfileFile.exists ())
        {
            qCWarning (lcUser) << "User profile file does not exist:"
                               << userProfilePath;
            return UserDataResult::FileNotFound;
        }

        if (!userProfileFile.open (QIODevice::ReadOnly))
        {
            qCWarning (lcUser) << "Failed to open user profile file:"
                               << userProfile;
            return UserDataResult::FileOpenError;
        }

        QByteArray fileData = userProfileFile.readAll ();
        userProfileFile.close ();

        if (fileData.isEmpty ())
        {
            qCWarning (lcUser) << "Failed to read user profile file or file "
                                  "is empty:"
                               << userProfile;
            return UserDataResult::FileReadError;
        }

        QJsonDocument userDataDoc = QJsonDocument::fromJson (fileData);
        if (userDataDoc.isNull () || !userDataDoc.isObject ())
        {
            qCWarning (lcUser) << "Failed to parse JSON from user profile "
                                  "file:"
                               << userProfile;
            return UserDataResult::InvalidData;
        }

        QJsonObject userData = userDataDoc.object ();

        auto validationResult = validateUserData (userData);
        if (!validationResult.isValid)
        {
            qCWarning (lcUser)
                << "User data is invalid:"
                << validationResult.ErrMessages.join (", ");
            return UserDataResult::InvalidData;
        }

        // Load user data
        QJsonObject appSettings = userData.value ("appSettings").toObject ();
        QString language = appSettings.value ("language").toString ();
        bool historyListEnabled =
            appSettings.value ("HistoryListEnabled").toBool ();

        if (language.isEmpty ())
        {
            qCWarning (lcUser)
                << "Language setting is missing in app settings data";
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
            qCWarning (lcUser)
                << "Failed to set app settings from user profile data";
            return UserDataResult::SettingError;
        }

        // update UI
        return UserDataResult::Success;
    });
}

UserDataResult UserModel::createUserData (
    const QString &username,
    const IUserProfileContextProvider &profileContextProvider)
{
    return guarded (lcUser, "creating user data",
                    UserDataResult::UnknownError,
                    [&]() -> UserDataResult
    {
        QJsonObject userData;

        const QString userUuid = profileContextProvider.getUserUuid (username);
        if (userUuid.isEmpty ())
        {
            qCWarning (lcUser) << "Cannot create user data because user UUID "
                                  "is empty";
            return UserDataResult::InvalidData;
        }

        const QString language = profileContextProvider.getLanguage ();
        if (language.isEmpty ())
        {
            qCWarning (lcUser)
                << "Cannot create user data because language is empty";
            return UserDataResult::InvalidData;
        }

        const QString filename = "profile_" + userUuid + ".json";

        QJsonObject userAccount;
        userAccount["username"] = username;
        userAccount["user_uuid"] = userUuid;

        userData["userAccount"] = userAccount;

        QJsonObject appSettings;
        appSettings["language"] = language;
        appSettings["HistoryListEnabled"] =
            AppSettingModel::getInstance ().isHistoryListEnabled ();
        userData["appSettings"] = appSettings;

        if (m_userProfileDir.isEmpty ())
        {
            qCWarning (lcUser) << "User profile directory is not set";
            return UserDataResult::EmptyPath;
        }

        QDir dir (m_userProfileDir);
        if (!dir.exists ())
        {
            if (!dir.mkpath ("."))
            {
                qCWarning (lcUser)
                    << "Failed to create user profile directory:"
                    << m_userProfileDir;
                return UserDataResult::DirectoryCreateError;
            }
        }

        QFile file (dir.filePath (filename));

        if (!file.open (QIODevice::WriteOnly))
        {
            qCWarning (lcUser) << "Failed to open file for writing:"
                               << filename;
            return UserDataResult::FileOpenError;
        }

        QJsonDocument doc (userData);
        if (file.write (doc.toJson ()) == -1)
        {
            file.close ();
            qCWarning (lcUser) << "Failed to write data to file:" << filename;
            return UserDataResult::FileWriteError;
        }
        file.close ();
        qCDebug (lcUser) << "User data saved to" << filename;
        return UserDataResult::Success;
    });
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
            qCWarning (lcUser) << "User profile file path is empty";
            return ChangeResult::NullValue;
        }

        QDir userProfileDir = QDir (getUserProfileDir ());
        if (!userProfileDir.exists ())
        {
            qCWarning (lcUser) << "User profile directory does not exist:"
                               << userProfileDir.path ();
            return ChangeResult::DirectoryNotFound;
        }

        QString userProfilePath = userProfileDir.filePath (userProfile);

        QFile userProfileFile (userProfilePath);
        if (!userProfileFile.exists ())
        {
            qCWarning (lcUser) << "User profile file does not exist:"
                               << userProfilePath;
            return ChangeResult::FileNotFound;
        }

        if (!userProfileFile.open (QIODevice::ReadOnly))
        {
            qCWarning (lcUser) << "Failed to open user profile file:"
                               << userProfilePath;
            return ChangeResult::FileOpenError;
        }

        QByteArray fileData = userProfileFile.readAll ();
        userProfileFile.close ();

        if (fileData.isEmpty ())
        {
            qCWarning (lcUser) << "Failed to read user profile file or file "
                                  "is empty:"
                               << userProfile;
            return ChangeResult::NullValue;
        }

        QJsonDocument userDataDoc = QJsonDocument::fromJson (fileData);
        if (userDataDoc.isNull () || !userDataDoc.isObject ())
        {
            qCWarning (lcUser) << "Failed to parse JSON from user profile "
                                  "file:"
                               << userProfilePath;
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
            qCWarning (lcUser)
                << "Failed to open user profile file for writing:"
                << userProfilePath;
            return ChangeResult::FileOpenError;
        }

        if (userProfileFile.write (updatedDoc.toJson ()) == -1)
        {
            userProfileFile.close ();
            qCWarning (lcUser)
                << "Failed to write data to user profile file:"
                << userProfilePath;
            return ChangeResult::FileWriteError;
        }

        userProfileFile.close ();
        return ChangeResult::Success;
    }
    catch (const std::exception &e)
    {
        qCCritical (lcUser) << "Error changing user profile field:" << field
                            << "Exception:" << e.what ();
        return ChangeResult::UnknownError;
    }
    catch (...)
    {
        qCCritical (lcUser) << "Error changing user profile field:" << field
                            << "Exception: Unknown exception";
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

ChangeResult UserModel::changeAiModeEnabled_Json (bool enabled,
                                                  const QString &userProfile)
{
    return changeUserProfileField (
        Constants::JsonFields::appSettings::AI_MODE_ENABLED, enabled,
        userProfile);
}

ChangeResult UserModel::changeLanguage_Json (const QString &lang,
                                             const QString &userProfile)
{
    return changeUserProfileField (Constants::JsonFields::appSettings::LANGUAGE,
                                   lang, userProfile);
}
