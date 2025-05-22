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
