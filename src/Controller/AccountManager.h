#pragma once
#include "Controller/DbManager.h"
#include "Model/AppSettingModel.h"
#include "Model/DbModel.h"
#include "Model/UserModel.h"
#include "Utility/Result.h"
#include <QString>
#include <stdexcept>

class AccountManager : public QObject
{
    Q_OBJECT

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
            logErr ("Error setting UserModel", e);
            throw;
        }
        catch (...)
        {
            logErr ("Unknown error setting UserModel",
                    std::runtime_error ("Unknown exception"));
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

    ChangeResult changeUsername (const QString &newUsername);
    ChangeResult changePassword (const QString &oldPasswordHash,
                                 const QString &newPasswordHash);

    ChangeResult changeEmail (const QString &newEmail);
    ChangeResult changeAvatar ();

    QString getUserUuid (const QString &username) const;
    QString getUsername () const;
    QString getHashedPassword () const;
    QString getEmail () const;
    QString getLanguage () const;
    QString getAvatarPath () const;

signals:
    void loginSuccessful (const QString &username);
    void logoutSuccessful ();
    void changeAvatarSuccessful (const QString &avatarPath);

private:
    AccountManager () = default;

    mutable std::string m_lastError;

    template <typename ExceptionT>
    void logErr (const std::string &errMsg, const ExceptionT &e) const
    {
        qCritical () << "[AccountManager]" << QString::fromStdString (errMsg)
                     << "Exception:" << e.what ();
        m_lastError = errMsg + " Exception:" + std::string (e.what ());
    }

    UserModel *userModel = nullptr;
    QString username{};
    QString password_Hash{};
    QString email{};
    QString avatarPath{};
};