#pragma once
#include <QJsonObject>
#include <QStringList>
#include <string>
#include <unordered_map>
enum class UserAuthResult
{
    Success,
    UserAlreadyLoggedIn,
    UserNotFound,
    IncorrectPassword,
    UserAlreadyExists,
    StorageError,
    UnknownError
};

static const std::unordered_map<UserAuthResult, std::string>
    UserAuthResultMessage = {
        {UserAuthResult::Success, "Login successful."},
        {UserAuthResult::UserAlreadyLoggedIn, "User already logged in."},
        {UserAuthResult::UserNotFound, "User not found."},
        {UserAuthResult::IncorrectPassword, "Incorrect password."},
        {UserAuthResult::UserAlreadyExists, "User already exists."},
        {UserAuthResult::StorageError, "Storage error."},
        {UserAuthResult::UnknownError, "Unknown error."}

};

enum class RegisterUserResult
{
    Success,
    UserAlreadyExists,
    DatabaseError,
    UnknownError
};

static const std::unordered_map<RegisterUserResult, std::string>
    RegisterUserResultMessage = {
        {RegisterUserResult::Success, "Registration successful."},
        {RegisterUserResult::UserAlreadyExists, "User already exists."},
        {RegisterUserResult::DatabaseError, "Database error."},
        {RegisterUserResult::UnknownError, "Unknown error."}

};

struct ValidationResult
{
    bool isValid;
    QStringList ErrMessages;
};

enum class ChangeResult
{
    Success,
    UserNotLoggedIn,
    AlreadyExists,
    StillSame,
    NullValue,
    FileNotFound,
    DatabaseError,
    UnknownError,
    Password_OldIncorrect
};

static const std::unordered_map<ChangeResult, std::string> ChangeResultMessage =
    {{ChangeResult::Success, "Change successful."},
     {ChangeResult::UserNotLoggedIn, "User not logged in."},
     {ChangeResult::AlreadyExists, "Username or email already exists."},
     {ChangeResult::StillSame, "New value is the same as the old one."},
     {ChangeResult::NullValue, "New value cannot be null."},
     {ChangeResult::FileNotFound, "File not found."},
     {ChangeResult::DatabaseError, "Database error."},
     {ChangeResult::UnknownError, "Unknown error."},
     {ChangeResult::Password_OldIncorrect, "Old password is incorrect."}

};