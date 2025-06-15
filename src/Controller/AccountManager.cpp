#include "AccountManager.h"
#include "DbManager.h"
#include "UserModel.h"
#include "Utility/Result.h"
#include <qcontainerfwd.h>

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
        }

        return result;
    }
    else
    {
        qCritical () << "UserModel is not set in AccountManager.";
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

            auto it = RegisterUserResultMessage.find (result);
            if (it != RegisterUserResultMessage.end ())
            {
                qCritical () << "User registration failed:"
                             << QString::fromStdString (it->second);
            }
            else
            {
                qCritical () << "User registration failed with unknown reason.";
            }
        }

        return result;
    }
    catch (const std::exception &e)
    {
        qCritical () << "Error registering user:" << e.what ();
        return RegisterUserResult::UnknownError;
    }
}

/*
invoke Example:

auto result = AccountManager::getInstance().registerUser(username, password);
if (result != RegisterUserResult::Success) {
    auto it = RegisterUserResultMessage.find(result);
    QString errorMsg = it != RegisterUserResultMessage.end()
                      ? QString::fromStdString(it->second)
                      : "Unknown error";
    // 在 UI 中显示错误信息
    showDialog(errorMsg);


*/
QString AccountManager::hashPassword (const QString &password)
{
    QByteArray byteArray = password.toUtf8 ();
    QByteArray hashed =
        QCryptographicHash::hash (byteArray, QCryptographicHash::Sha256);

    return QString (hashed.toHex ());
}

ChangeResult AccountManager::changeUsername (const QString &newUsername)
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

ChangeResult AccountManager::changePassword (const QString &oldPasswordHash,
                                             const QString &newPasswordHash)
{
    auto result = DbManager::getInstance ().changePassword (
        username, oldPasswordHash, newPasswordHash);

    if (result == ChangeResult::Success)
    {
        password_Hash = newPasswordHash;
    }

    return result;
}

ChangeResult AccountManager::changeEmail (const QString &newEmail)
{

    auto result =
        DbManager::getInstance ().changeEmail (username, email, newEmail);

    if (result == ChangeResult::Success)
    {
        email = newEmail;
    }

    return result;
}

QString AccountManager::getUsername () const { return username; }

QString AccountManager::getHashedPassword () const { return password_Hash; }

QString AccountManager::getEmail () const { return email; }

QString AccountManager::getLanguage () const
{
    return AppSettingModel::getInstance ().getLanguage ();
}

QString AccountManager::getAvatarPath () const { return avatarPath; }
