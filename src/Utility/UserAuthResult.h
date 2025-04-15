#pragma once

enum class UserAuthResult
{
    Success,
    UserNotFound,
    IncorrectPassword,
    UserAlreadyExists,
    StorageError,
    UnknownError
};