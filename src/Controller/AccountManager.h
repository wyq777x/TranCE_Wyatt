#pragma once
#include "Model/AppSettingModel.h"
#include "Model/DictDbModel.h"
#include "Model/UserModel.h"
#include "Utility/UserAuthResult.h"
#include <QString>
#include <stdexcept>
class AccountManager
{

public:
    static AccountManager &getInstance ()
    {
        static AccountManager instance;
        return instance;
    }

    AccountManager (const AccountManager &) = delete;
    AccountManager &operator= (const AccountManager &) = delete;
    AccountManager (AccountManager &&) = delete;
    AccountManager &operator= (AccountManager &&) = delete;

    void login (const QString &username, const QString &password);
    void logout ();
    void registerUser (const QString &username, const QString &password);

    bool isLoggedIn () const { return UserModel::getInstance ().isLoggedIn (); }
    bool isLoginExpired () const
    {
        return UserModel::getInstance ().isLoginExpired ();
    }

    void setUsername (const QString &username);
    static QString hashPassword (const QString &password);
    QString getUsername () const;
    QString getHashedPassword () const;
    QString getLanguage () const;

private:
    AccountManager () = default;

    void setPassword (const QString &password);

    QString username;
    QString password_Hash;
};