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

enum class UserDataResult
{
    Success,
    EmptyPath,
    DirectoryCreateError,
    DirectoryNotFound,
    FileOpenError,
    FileReadError,
    FileWriteError,
    FileDeleteError,
    FileNotFound,
    InvalidData,
    UnknownError
};

static const std::unordered_map<UserDataResult, std::string>
    UserDataResultMessage = {
        {UserDataResult::Success, "Operation successful."},
        {UserDataResult::EmptyPath, "Path cannot be empty."},
        {UserDataResult::DirectoryCreateError, "Error creating directory."},
        {UserDataResult::DirectoryNotFound, "Directory not found."},
        {UserDataResult::FileOpenError, "Error opening file."},
        {UserDataResult::FileReadError, "Error reading file."},
        {UserDataResult::FileWriteError, "Error writing to file."},
        {UserDataResult::FileDeleteError, "Error deleting file."},
        {UserDataResult::FileNotFound, "File not found."},
        {UserDataResult::InvalidData, "Invalid data provided."},
        {UserDataResult::UnknownError, "Unknown error occurred."}

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
    Password_OldIncorrect,
    InvalidInput,
    FileOpenError,
    FileWriteError
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
     {ChangeResult::Password_OldIncorrect, "Old password is incorrect."},
     {ChangeResult::InvalidInput, "Invalid input provided."},
     {ChangeResult::FileOpenError, "Error opening file."},
     {ChangeResult::FileWriteError, "Error writing to file."}

};

template <typename ResultT>
std::string getErrorMessage (
    ResultT result,
    const std::unordered_map<ResultT, std::string> &ResultMessageMap)
{
    auto it = ResultMessageMap.find (result);

    return it != ResultMessageMap.end () ? it->second : "Unknown error.";
}
