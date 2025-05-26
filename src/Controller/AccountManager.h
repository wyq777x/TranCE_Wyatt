#pragma once
#include "Model/AppSettingModel.h"
#include "Model/DbModel.h"
#include "Model/UserModel.h"
#include "Utility/Result.h"
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

    void setUserModel (UserModel *model)
    {
        try
        {
            if (model == nullptr)
                throw std::runtime_error ("UserModel is null");

            userModel = model;
        }
        catch (const std::exception &e)
        {
            qDebug () << "Error setting UserModel:" << e.what ();
            throw;
        }
    }
    UserAuthResult login (const QString &username, const QString &password);
    void logout ();
    RegisterUserResult registerUser (const QString &username,
                                     const QString &password);
    static QString hashPassword (const QString &password);

    bool isLoggedIn () const { return UserModel::getInstance ().isLoggedIn (); }
    bool isLoginExpired () const
    {
        return UserModel::getInstance ().isLoginExpired ();
    }

    void setUsername (const QString &username);
    QString getUsername () const;
    QString getHashedPassword () const;
    QString getLanguage () const;

private:
    AccountManager () = default;

    UserModel *userModel = nullptr;
    QString username;
    QString password_Hash;
};