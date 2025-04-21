#include "AccountManager.h"
#include "UserModel.h"
#include "Utility/UserAuthResult.h"

void AccountManager::logout ()
{

    auto &instance = UserModel::getInstance ();
    instance.logout ();
    instance.setLoggedIn (false);
    instance.setLoginExpired (true);
    // clear the username and password of View
    // ...
}

void AccountManager::login (const QString &username, const QString &password)
{
    auto &instance = UserModel::getInstance ();

    auto result = instance.login (username, password);

    return;
}

void AccountManager::registerUser (const QString &username,
                                   const QString &password)
{
    auto &instance = UserModel::getInstance ();

    return;
}