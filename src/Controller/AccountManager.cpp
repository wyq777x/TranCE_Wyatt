#include "AccountManager.h"
#include "UserModel.h"

void AccountManager::logout ()
{

    UserModel::getInstance ().logout ();

    // clear the username and password of View
    // ...
}
