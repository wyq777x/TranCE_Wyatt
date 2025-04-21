#include "AccountManager.h"
#include "UserModel.h"

void AccountManager::logout ()
{

    UserModel::getInstance ().logout ();

    // clear the username and password of View
    // ...
}

void AccountManager::login (const QString &username, const QString &password)
{

    return;
}

void AccountManager::registerUser (const QString &username,
                                   const QString &password)
{
    return;
}