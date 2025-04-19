#pragma once
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

    bool isLoggedIn () const;
    QString getUsername () const;
    void setUsername (const QString &username);

private:
    AccountManager () = default;

    void setPassword (const QString &password);
    QString getPassword () const;

    QString username;
    QString password_Hash;
};