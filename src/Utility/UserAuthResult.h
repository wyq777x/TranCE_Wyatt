#pragma once
#include <string>
#include <unordered_map>
enum class UserAuthResult
{
    Success,
    UserNotFound,
    IncorrectPassword,
    UserAlreadyExists,
    StorageError,
    UnknownError
};

static const std::unordered_map<UserAuthResult, std::string>
    UserAuthResultMessage = {
        {UserAuthResult::Success, "Login successful."},
        {UserAuthResult::UserNotFound, "User not found."},
        {UserAuthResult::IncorrectPassword, "Incorrect password."},
        {UserAuthResult::UserAlreadyExists, "User already exists."},
        {UserAuthResult::StorageError, "Storage error."},
        {UserAuthResult::UnknownError, "Unknown error."}

};
