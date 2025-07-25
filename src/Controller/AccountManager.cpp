#include "AccountManager.h"
#include "Controller/DbManager.h"
#include "Model/AppSettingModel.h"
#include "Utility/Result.h"
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QRegularExpression>
#include <QStandardPaths>

void AccountManager::logout ()
{

    if (userModel)
        userModel->logout ();

    username.clear ();
    password_Hash.clear ();
    email.clear ();
    avatarPath.clear ();
}

UserAuthResult AccountManager::login (const QString &username,
                                      const QString &password)
{
    try
    {
        if (userModel)
        {
            auto result = userModel->login (username, password);
            if (result == UserAuthResult::Success)
            {
                this->username = username;
                this->password_Hash = hashPassword (password);
                this->email = DbManager::getInstance ().getUserEmail (username);
                this->avatarPath =
                    DbManager::getInstance ().getUserAvatarPath (username);
                emit loginSuccessful (username);

                auto loadUserDataResult = userModel->loadUserData (
                    "profile_" +
                    AccountManager::getInstance ().getUserUuid (
                        AccountManager::getInstance ().getUsername ()) +
                    ".json");
                if (loadUserDataResult != UserDataResult::Success)
                {
                    logErr ("Failed to load user data after login",
                            std::runtime_error (
                                "Failed to load user data after login: " +
                                getErrorMessage (loadUserDataResult,
                                                 UserDataResultMessage)));
                }
            }

            return result;
        }
        else
        {
            auto exception =
                std::runtime_error ("UserModel is not set in AccountManager");
            logErr ("UserModel not set", exception);
            return UserAuthResult::UnknownError;
        }
    }
    catch (const std::exception &e)
    {
        logErr ("Error during login process", e);
        return UserAuthResult::UnknownError;
    }
    catch (...)
    {
        logErr ("Unknown error during login process",
                std::runtime_error ("Unknown exception"));
        return UserAuthResult::UnknownError;
    }
}

RegisterUserResult AccountManager::registerUser (const QString &username,
                                                 const QString &password)
{
    try
    {
        password_Hash = hashPassword (password);
        auto result =
            DbManager::getInstance ().registerUser (username, password_Hash);

        if (result != RegisterUserResult::Success)
        {

            std::string errorMsg =
                "User registration failed: " +
                getErrorMessage (result, RegisterUserResultMessage);

            auto exception = std::runtime_error (errorMsg);
            logErr ("User registration failed", exception);
        }

        return result;
    }
    catch (const std::exception &e)
    {
        logErr ("Error registering user", e);
        return RegisterUserResult::UnknownError;
    }
    catch (...)
    {
        logErr ("Unknown error registering user",
                std::runtime_error ("Unknown exception"));
        return RegisterUserResult::UnknownError;
    }
}

QString AccountManager::hashPassword (const QString &password)
{
    QByteArray byteArray = password.toUtf8 ();
    QByteArray hashed =
        QCryptographicHash::hash (byteArray, QCryptographicHash::Sha256);

    return QString (hashed.toHex ());
}

ChangeResult AccountManager::changeUsername (const QString &newUsername)
{
    try
    {
        if (newUsername.isEmpty ())
        {
            return ChangeResult::NullValue;
        }

        auto result =
            DbManager::getInstance ().changeUsername (username, newUsername);

        if (result == ChangeResult::Success)
        {
            username = newUsername;
        }

        return result;
    }
    catch (const std::exception &e)
    {
        logErr ("Error changing username", e);
        return ChangeResult::DatabaseError;
    }
    catch (...)
    {
        logErr ("Unknown error changing username",
                std::runtime_error ("Unknown exception"));
        return ChangeResult::UnknownError;
    }
}

ChangeResult AccountManager::changePassword (const QString &oldPasswordHash,
                                             const QString &newPasswordHash)
{
    try
    {
        auto result = DbManager::getInstance ().changePassword (
            username, oldPasswordHash, newPasswordHash);

        if (result == ChangeResult::Success)
        {
            password_Hash = newPasswordHash;
        }

        return result;
    }
    catch (const std::exception &e)
    {
        logErr ("Error changing password", e);
        return ChangeResult::DatabaseError;
    }
    catch (...)
    {
        logErr ("Unknown error changing password",
                std::runtime_error ("Unknown exception"));
        return ChangeResult::UnknownError;
    }
}

ChangeResult AccountManager::changeEmail (const QString &newEmail)
{
    try
    {
        auto result =
            DbManager::getInstance ().changeEmail (username, email, newEmail);

        if (result == ChangeResult::Success)
        {
            email = newEmail;
        }

        return result;
    }
    catch (const std::exception &e)
    {
        logErr ("Error changing email", e);
        return ChangeResult::DatabaseError;
    }
    catch (...)
    {
        logErr ("Unknown error changing email",
                std::runtime_error ("Unknown exception"));
        return ChangeResult::UnknownError;
    }
}

ChangeResult AccountManager::changeAvatar ()
{
    QString filePath = QString ();

    filePath = QFileDialog::getOpenFileName (
        nullptr, "Choose Avatar", "", "Images(*.png *.jpg *.bmp *.jpeg *gif)");

    if (filePath.isEmpty ())
    {
        qDebug () << "No file selected.";
        return ChangeResult::NullValue;
    }
    else
    {
        qDebug () << "Selected file:" << filePath;
    }

    if (!QFile::exists (filePath))
    {
        auto exception = std::runtime_error ("Avatar file does not exist: " +
                                             filePath.toStdString ());
        logErr ("Avatar file not found", exception);
        return ChangeResult::FileNotFound;
    }

    QString avatarDir =
        QCoreApplication::applicationDirPath () + "/Utility/Avatars";
    QDir dir;
    if (!dir.exists (avatarDir))
    {
        if (!dir.mkpath (avatarDir))
        {
            auto exception =
                std::runtime_error ("Failed to create avatar directory: " +
                                    avatarDir.toStdString ());
            logErr ("Failed to create avatar directory", exception);
            return ChangeResult::DatabaseError;
        }
    }

    QFileInfo fileInfo (filePath);
    QString extension = fileInfo.suffix ();

    QDir avatarDirHandle (avatarDir);
    QStringList nameFilters;
    nameFilters << QString ("%1-*").arg (username);
    QStringList existingAvatars =
        avatarDirHandle.entryList (nameFilters, QDir::Files);

    int nextNumber = 1;
    if (!existingAvatars.isEmpty ())
    {
        QStringList numbers;
        for (const QString &filename : existingAvatars)
        {
            QRegularExpression regex (QString ("%1-(\\d+)").arg (username));
            QRegularExpressionMatch match = regex.match (filename);
            if (match.hasMatch ())
            {
                numbers.append (match.captured (1));
            }
        }

        if (!numbers.isEmpty ())
        {
            std::sort (numbers.begin (), numbers.end (),
                       [] (const QString &a, const QString &b)
                       { return a.toInt () < b.toInt (); });
            nextNumber = numbers.last ().toInt () + 1;
        }
    }

    QString newFileName =
        QString ("%1-%2.%3").arg (username).arg (nextNumber).arg (extension);
    QString newAvatarPath = avatarDir + "/" + newFileName;

    if (!QFile::copy (filePath, newAvatarPath))
    {
        auto exception = std::runtime_error (
            "Failed to copy avatar file from " + filePath.toStdString () +
            " to " + newAvatarPath.toStdString ());
        logErr ("Failed to copy avatar file", exception);
        return ChangeResult::DatabaseError;
    }

    auto result =
        DbManager::getInstance ().changeAvatar (username, newAvatarPath);

    if (result == ChangeResult::Success)
    {
        this->avatarPath = newAvatarPath;
        emit changeAvatarSuccessful (newAvatarPath);
        qDebug () << "Avatar changed successfully to" << newAvatarPath;
    }

    return result;
}

ChangeResult
AccountManager::changeHistorySearchListEnabled_Json (bool enabled,
                                                     const QString &userProfile)
{

    auto result =
        UserModel::getInstance ().changeHistorySearchListEnabled_Json (
            enabled, userProfile);

    return result;
}

ChangeResult AccountManager::changeLanguage_Json (const QString &lang,
                                                  const QString &userProfile)
{
    auto result =
        UserModel::getInstance ().changeLanguage_Json (lang, userProfile);

    return result;
}

QString AccountManager::getUserUuid (const QString &username) const
{
    auto UUID = DbManager::getInstance ().getUserId (username);

    if (!UUID.has_value ())
    {
        auto exception = std::runtime_error (
            "User UUID not found for username: " + username.toStdString ());
        logErr ("User UUID not found", exception);
        return QString ();
    }

    return UUID.value ();
}

QString AccountManager::getUsername () const { return username; }

QString AccountManager::getHashedPassword () const { return password_Hash; }

QString AccountManager::getEmail () const { return email; }

QString AccountManager::getLanguage () const
{
    return AppSettingModel::getInstance ().getLanguage ();
}

QString AccountManager::getAvatarPath () const { return avatarPath; }
