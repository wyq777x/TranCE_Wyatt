#pragma once
#include "Utility/IUserProfileContextProvider.h"
#include "Utility/Result.h"
#include <QDebug>
#include <QObject>
#include <QString>
#include <stdexcept>

class UserModel;

class AccountManager : public QObject, public IUserProfileContextProvider
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
    static bool verifyPassword (const QString &password,
                                const QString &storedHash);

    bool isLoggedIn () const;
    bool isLoginExpired () const;

    UserDataResult createUserData (const QString &username);

    ChangeResult changeUsername (const QString &newUsername);

    ChangeResult changePassword (const QString &oldPassword,
                                 const QString &newPassword);

    ChangeResult changeEmail (const QString &newEmail);

    ChangeResult changeAvatar ();

    ChangeResult
    changeHistorySearchListEnabled_Json (bool enabled,
                                         const QString &userProfile);

    ChangeResult changeLanguage_Json (const QString &lang,
                                      const QString &userProfile);

    QString getUserUuid (const QString &username) const override;
    QString getUsername () const;
    QString getHashedPassword () const;
    QString getEmail () const;
    QString getLanguage () const override;
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
