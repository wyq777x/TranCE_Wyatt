#pragma once

#include <QString>

class IUserProfileContextProvider
{
public:
    virtual ~IUserProfileContextProvider () = default;

    virtual QString getUserUuid (const QString &username) const = 0;
    virtual QString getLanguage () const = 0;
};
