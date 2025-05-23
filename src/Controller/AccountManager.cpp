#include "AccountManager.h"
#include "DbModel.h"
#include "UserModel.h"
#include "Utility/Result.h"
#include <qcontainerfwd.h>

void AccountManager::logout ()
{

    if (userModel)
        userModel->logout ();
    username.clear ();
    password_Hash.clear ();
}

void AccountManager::login (const QString &username, const QString &password)
{
    if (userModel)
    {
        auto result = userModel->login (username, password);
        if (result == UserAuthResult::Success)
        {
            this->username = username;
            this->password_Hash = hashPassword (password);
        }
    }
}

RegisterUserResult AccountManager::registerUser (const QString &username,
                                                 const QString &password)
{
    try
    {
        password_Hash = hashPassword (password);
        auto result = DbModel::registerUser (username, password_Hash);

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
                qCritical ()
                    << "User registration failed with unknown result code";
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

auto result = AccountManager->getInstance().registerUser(username, password);
if (result != RegisterUserResult::Success) {
    auto it = RegisterUserResultMessage.find(result);
    QString errorMsg = it != RegisterUserResultMessage.end()
                      ? QString::fromStdString(it->second)
                      : "Unknown error";
    // 在 UI 中显示错误信息
    showErrorDialog(errorMsg);


*/
QString AccountManager::hashPassword (const QString &password)
{
    QByteArray byteArray = password.toUtf8 ();
    QByteArray hashed =
        QCryptographicHash::hash (byteArray, QCryptographicHash::Sha256);

    return QString (hashed.toHex ());
}

QString AccountManager::getUsername () const { return username; }

QString AccountManager::getHashedPassword () const { return password_Hash; }

QString AccountManager::getLanguage () const
{
    return AppSettingModel::getInstance ().getLanguage ();
}