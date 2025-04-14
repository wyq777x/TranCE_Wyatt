#pragma once

class UserModel
{

public:
    static UserModel &getInstance ()
    {
        static UserModel instance;
        return instance;
    }

    UserModel (const UserModel &) = delete;
    UserModel &operator= (const UserModel &) = delete;
    UserModel (UserModel &&) = delete;
    UserModel &operator= (UserModel &&) = delete;

private:
    UserModel () = default;
};