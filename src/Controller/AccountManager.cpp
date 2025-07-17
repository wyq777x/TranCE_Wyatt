#include "AccountManager.h"
#include "DbManager.h"
#include "UserModel.h"
#include "Utility/Result.h"
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QRegularExpression>
#include <QStandardPaths>
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
    showDialog(errorMsg);}


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
        qDebug () << "Avatar file does not exist:" << filePath;
        return ChangeResult::FileNotFound;
    }

    QString avatarDir =
        QCoreApplication::applicationDirPath () + "/Utility/Avatars";
    QDir dir;
    if (!dir.exists (avatarDir))
    {
        if (!dir.mkpath (avatarDir))
        {
            qDebug () << "Failed to create avatar directory:" << avatarDir;
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
        qDebug () << "Failed to copy avatar file from" << filePath << "to"
                  << newAvatarPath;
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

QString AccountManager::getUserUuid () const
{
    auto UUID = DbManager::getInstance ().getUserId (username);

    if (!UUID.has_value ())
    {
        qWarning () << "User UUID not found for username:" << username;
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
