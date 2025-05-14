#include "DbModel.h"

/*

UserAuthResult UserModel::Authenticate (const QString &username,
                                        const QString &password)
{
    QJsonArray userList = getUserList ("users.json");

    for (const QJsonValue &userVal : userList)
    {
        QJsonObject userObj = userVal.toObject ();
        QString storedUsername = userObj["username"].toString ();
        QString storedPasswordHash = userObj["password"].toString ();

        if (username == storedUsername)
        {
            QString inputPasswordHash = AccountManager::hashPassword (password);
            if (inputPasswordHash == storedPasswordHash)
                return UserAuthResult::Success;
            else
                return UserAuthResult::IncorrectPassword;
        }
    }
    return UserAuthResult::UserNotFound;
}
*/