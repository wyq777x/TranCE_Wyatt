#include "AccountManager.h"
#include "Controller/DbManager.h"
#include "Model/AppSettingModel.h"
#include "Model/UserModel.h"
#include "Utility/AspectGuard.h"
#include "Utility/Logging.h"
#include "Utility/PasswordHasher.h"
#include "Utility/Result.h"
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QtConcurrent/QtConcurrent>

using trance::aspect::guarded;

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
    return guarded (lcAccount, "during login process",
                    UserAuthResult::UnknownError,
                    [&]() -> UserAuthResult
    {
        if (userModel)
        {
            auto result = userModel->login (username, password);
            if (result == UserAuthResult::Success)
            {
                this->username = username;
                this->password_Hash =
                    DbManager::getInstance ().getUserPasswordHash (username);
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
                    qCWarning (lcAccount)
                        << "Failed to load user data after login:"
                        << QString::fromStdString (getErrorMessage (
                               loadUserDataResult, UserDataResultMessage));
                }
            }

            return result;
        }
        else
        {
            qCWarning (lcAccount) << "UserModel not set";
            return UserAuthResult::UnknownError;
        }
    });
}

QFuture<UserAuthResult>
AccountManager::loginAsync (const QString &username, const QString &password)
{
    // login() touches DbModel (SQLite) and this manager's member state; both
    // live on the GUI thread, so we keep the DB work on the GUI thread and
    // only move the expensive PBKDF2 derivation off it: fetch the stored
    // hash, verify off-thread, then finish the login synchronously.
    return QtConcurrent::run (
        [username, password] () -> UserAuthResult
        { return AccountManager::getInstance ().login (username, password); });
}

QFuture<RegisterUserResult>
AccountManager::registerUserAsync (const QString &username,
                                   const QString &password)
{
    return QtConcurrent::run (
        [username, password] () -> RegisterUserResult
        {
            return AccountManager::getInstance ().registerUser (username,
                                                                password);
        });
}

QFuture<ChangeResult>
AccountManager::changePasswordAsync (const QString &oldPassword,
                                     const QString &newPassword)
{
    return QtConcurrent::run (
        [this, oldPassword, newPassword] () -> ChangeResult
        { return changePassword (oldPassword, newPassword); });
}

RegisterUserResult AccountManager::registerUser (const QString &username,
                                                 const QString &password)
{
    return guarded (lcAccount, "registering user",
                    RegisterUserResult::UnknownError,
                    [&]() -> RegisterUserResult
    {
        password_Hash = hashPassword (password);
        auto result =
            DbManager::getInstance ().registerUser (username, password_Hash);

        if (result != RegisterUserResult::Success)
        {
            qCWarning (lcAccount)
                << "User registration failed:"
                << QString::fromStdString (
                       getErrorMessage (result, RegisterUserResultMessage));
        }

        return result;
    });
}

QString AccountManager::hashPassword (const QString &password)
{
    return PasswordHasher::hashPassword (password);
}

bool AccountManager::verifyPassword (const QString &password,
                                     const QString &storedHash)
{
    return PasswordHasher::verifyPassword (password, storedHash);
}

UserDataResult AccountManager::createUserData (const QString &username)
{
    if (userModel == nullptr)
    {
        qCWarning (lcAccount) << "UserModel not set";
        return UserDataResult::UnknownError;
    }

    auto result = userModel->createUserData (username, *this);

    return result;
}

bool AccountManager::isLoggedIn () const
{
    return userModel != nullptr && userModel->isLoggedIn ();
}

bool AccountManager::isLoginExpired () const
{
    return userModel == nullptr || userModel->isLoginExpired ();
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
        trance::log::logException (lcAccount, "Error changing username", e);
        return ChangeResult::DatabaseError;
    }
    catch (...)
    {
        trance::log::logExceptionUnknown (lcAccount, "Error changing username");
        return ChangeResult::UnknownError;
    }
}

ChangeResult AccountManager::changePassword (const QString &oldPassword,
                                             const QString &newPassword)
{
    try
    {
        auto result = DbManager::getInstance ().changePassword (
            username, oldPassword, newPassword);

        if (result == ChangeResult::Success)
        {
            password_Hash =
                DbManager::getInstance ().getUserPasswordHash (username);
        }

        return result;
    }
    catch (const std::exception &e)
    {
        trance::log::logException (lcAccount, "Error changing password", e);
        return ChangeResult::DatabaseError;
    }
    catch (...)
    {
        trance::log::logExceptionUnknown (lcAccount, "Error changing password");
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
        trance::log::logException (lcAccount, "Error changing email", e);
        return ChangeResult::DatabaseError;
    }
    catch (...)
    {
        trance::log::logExceptionUnknown (lcAccount, "Error changing email");
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
        qCDebug (lcAccount) << "No file selected.";
        return ChangeResult::NullValue;
    }
    else
    {
        qCDebug (lcAccount) << "Selected file:" << filePath;
    }

    if (!QFile::exists (filePath))
    {
        qCWarning (lcAccount) << "Avatar file not found:" << filePath;
        return ChangeResult::FileNotFound;
    }

    QString avatarDir =
        QCoreApplication::applicationDirPath () + "/Utility/Avatars";
    QDir dir;
    if (!dir.exists (avatarDir))
    {
        if (!dir.mkpath (avatarDir))
        {
            qCWarning (lcAccount) << "Failed to create avatar directory:"
                                  << avatarDir;
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
        qCWarning (lcAccount) << "Failed to copy avatar file from" << filePath
                              << "to" << newAvatarPath;
        return ChangeResult::DatabaseError;
    }

    auto result =
        DbManager::getInstance ().changeAvatar (username, newAvatarPath);

    if (result == ChangeResult::Success)
    {
        this->avatarPath = newAvatarPath;
        emit changeAvatarSuccessful (newAvatarPath);
        qCDebug (lcAccount) << "Avatar changed successfully to"
                            << newAvatarPath;
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

ChangeResult AccountManager::changeAiModeEnabled_Json (bool enabled,
                                                       const QString &userProfile)
{
    auto result =
        UserModel::getInstance ().changeAiModeEnabled_Json (enabled,
                                                            userProfile);

    return result;
}

QString AccountManager::getUserUuid (const QString &username) const
{
    auto UUID = DbManager::getInstance ().getUserId (username);

    if (!UUID.has_value ())
    {
        qCWarning (lcAccount) << "User UUID not found for username:"
                              << username;
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
