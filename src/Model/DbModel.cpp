#include "DbModel.h"
#include "SQLiteCpp/Exception.h"
#include "SQLiteCpp/Transaction.h"
#include "Utility/Result.h"
#include <quuid.h>
#include <stdexcept>

bool DbModel::isUserDbOpen () const
{
    if (user_db)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool DbModel::isDictDbOpen () const
{
    if (dict_db)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool DbModel::userExists (const QString &username) const
{
    if (!isUserDbOpen ())
    {
        return false;
    }

    try
    {
        SQLite::Statement query (
            *user_db, "SELECT COUNT(*) FROM users WHERE username = ?");
        query.bind (1, username.toStdString ());
        query.executeStep ();
        return query.getColumn (0).getInt () > 0;
    }
    catch (const SQLite::Exception &e)
    {
        logErr ("Error checking if user exists", e);
        return false;
    }
    catch (const std::exception &e)
    {
        logErr ("Unknown error checking if user exists", e);
        return false;
    }
    catch (...)
    {
        logErr ("Unknown error checking if user exists",
                std::runtime_error ("Unknown exception"));
        return false;
    }
}

RegisterUserResult DbModel::registerUser (const QString &username,
                                          const QString &passwordHash)
{
    DbModel &instance = DbModel::getInstance ();
    if (!instance.isUserDbOpen ())
    {
        return RegisterUserResult::DatabaseError;
    }
    if (instance.userExists (username))
    {
        return RegisterUserResult::UserAlreadyExists;
    }
    try
    {
        QString userId = QUuid::createUuid ().toString (QUuid::WithoutBraces);

        SQLite::Statement query (
            *(instance.user_db),
            "INSERT INTO users (user_id, username, password_hash) "
            "VALUES (?, ?, ?)");
        query.bind (1, userId.toStdString ());
        query.bind (2, username.toStdString ());
        query.bind (3, passwordHash.toStdString ());
        query.exec ();

        // Check if the user was inserted successfully
        if (query.getChanges () == 0)
        {
            return RegisterUserResult::DatabaseError;
        }

        return RegisterUserResult::Success;
    }
    catch (const SQLite::Exception &e)
    {
        instance.logErr ("Error inserting user into database", e);
        return RegisterUserResult::DatabaseError;
    }
    catch (const std::exception &e)
    {
        instance.logErr ("Unknown error inserting user into database", e);
        return RegisterUserResult::UnknownError;
    }
    catch (...)
    {
        instance.logErr ("Unknown error inserting user into database",
                         std::runtime_error ("Unknown exception"));
        return RegisterUserResult::UnknownError;
    }
}

UserAuthResult DbModel::verifyUser (const QString &username,
                                    const QString &passwordHash) const
{

    try
    {
        if (!isUserDbOpen ())
        {
            return UserAuthResult::StorageError;
        }
        SQLite::Statement query (*user_db, "SELECT password_hash FROM users "
                                           "WHERE username = ?");
        query.bind (1, username.toStdString ());
        if (query.executeStep ())
        {
            std::string storedHash = query.getColumn (0).getString ();

            if (storedHash == passwordHash.toStdString ())
            {
                return UserAuthResult::Success;
            }
            else
            {
                return UserAuthResult::IncorrectPassword;
            }
        }
        else
        {
            return UserAuthResult::UserNotFound;
        }
    }
    catch (const SQLite::Exception &e)
    {
        logErr ("Error verifying user in database", e);
        return UserAuthResult::StorageError;
    }
    catch (const std::exception &e)
    {
        logErr ("Unknown error verifying user in database", e);
        return UserAuthResult::UnknownError;
    }
    catch (...)
    {
        logErr ("Unknown error verifying user in database",
                std::runtime_error ("Unknown exception"));
        return UserAuthResult::UnknownError;
    }
}

void DbModel::deleteUser (const QString &username)
{
    if (!isUserDbOpen ())
    {
        return;
    }

    try
    {
        SQLite::Statement query (*user_db,
                                 "DELETE FROM users WHERE username = ?");
        query.bind (1, username.toStdString ());
        query.exec ();
    }
    catch (const SQLite::Exception &e)
    {
        logErr ("Error deleting user from database", e);
    }
    catch (const std::exception &e)
    {
        logErr ("Unknown error deleting user from database", e);
    }
    catch (...)
    {
        logErr ("Unknown error deleting user from database",
                std::runtime_error ("Unknown exception"));
    }
}

void DbModel::updateUserPassword (const QString &username,
                                  const QString &oldPasswordHash,
                                  const QString &newPasswordHash)
{
    if (!isUserDbOpen ())
    {
        return;
    }

    try
    {
        SQLite::Statement query (*user_db,
                                 "UPDATE users SET password_hash = ? "
                                 "WHERE username = ? AND password_hash = ?");
        query.bind (1, newPasswordHash.toStdString ());
        query.bind (2, username.toStdString ());
        query.bind (3, oldPasswordHash.toStdString ());
        query.exec ();
    }
    catch (const SQLite::Exception &e)
    {
        logErr ("Error updating user password in database", e);
    }
    catch (const std::exception &e)
    {
        logErr ("Unknown error updating user password in database", e);
    }
    catch (...)
    {
        logErr ("Unknown error updating user password in database",
                std::runtime_error ("Unknown exception"));
    }
}

AsyncTask<void> DbModel::importWordEntry (const WordEntry &wordEntry)
{
    if (!isDictDbOpen ())
    {
        co_return;
    }

    try
    {

        // Insert word entry into the words table
        SQLite::Transaction transaction (*dict_db);

        SQLite::Statement wordsQuery (
            *dict_db, "INSERT INTO words "
                      "(word,part_of_speech,pronunciation,frequency,"
                      "notes) VALUES (?,?,?,?,?)");

        wordsQuery.bind (1, wordEntry.word.toStdString ());
        wordsQuery.bind (2, wordEntry.partOfSpeech.toStdString ());
        wordsQuery.bind (3, wordEntry.pronunciation.toStdString ());
        wordsQuery.bind (4, wordEntry.frequency);
        wordsQuery.bind (5, wordEntry.notes.toStdString ());
        wordsQuery.exec ();

        // Insert translations

        if (!wordEntry.translation.isEmpty ())
        {
            SQLite::Statement translationsQuery (
                *dict_db, "INSERT INTO word_translations"
                          "(source_word, source_language, "
                          "target_word, target_language,confidence_score) "
                          "VALUES (?,?,?,?,?)");
            translationsQuery.bind (1, wordEntry.word.toStdString ());
            translationsQuery.bind (2, wordEntry.language.toStdString ());
            translationsQuery.bind (3, wordEntry.translation.toStdString ());
            translationsQuery.bind (4, wordEntry.language.toStdString ());
            translationsQuery.bind (5, 1.0); // Default confidence score
            translationsQuery.exec ();
        }

        // Insert examples

        if (!wordEntry.examples.isEmpty ())
        {
            SQLite::Statement examplesQuery (*dict_db,
                                             "INSERT INTO examples (word, "
                                             "example_sentence,translation) "
                                             "VALUES (?,?,?,?)");
            examplesQuery.bind (1, wordEntry.word.toStdString ());
            examplesQuery.bind (2, wordEntry.examples.toStdString ());
            examplesQuery.bind (3, wordEntry.exampleTranslation.toStdString ());
            examplesQuery.exec ();
        }

        transaction.commit ();
        co_return;
    }
    catch (const SQLite::Exception &e)
    {
        logErr ("Error importing word entry", e);
    }
    catch (const std::exception &e)
    {
        logErr ("Unknown error importing word entry", e);
    }
    catch (...)
    {
        logErr ("Unknown error importing word entry",
                std::runtime_error ("Unknown exception"));
    }
}

AsyncTask<void>
DbModel::importWordEntriesAsync (const std::vector<WordEntry> &wordEntries)
{
    if (!isDictDbOpen ())
    {
        co_return;
    }

    try
    {

        // to be implemented
    }
    catch (const SQLite::Exception &e)
    {
        logErr ("Error importing word entries", e);
    }
    catch (const std::exception &e)
    {
        logErr ("Unknown error importing word entries", e);
    }
    catch (...)
    {
        logErr ("Unknown error importing word entries",
                std::runtime_error ("Unknown exception"));
    }
}

AsyncTask<void>
DbModel::importFromFileAsync (const QString &filePath,
                              std::function<void (int, int)> progressCallback)
{
    if (!isDictDbOpen ())
    {
        co_return;
    }

    try
    {
        // to be implemented
        co_return;
    }
    catch (const SQLite::Exception &e)
    {
        logErr ("Error importing from file", e);
    }
    catch (const std::exception &e)
    {
        logErr ("Unknown error importing from file", e);
    }
    catch (...)
    {
        logErr ("Unknown error importing from file",
                std::runtime_error ("Unknown exception"));
    }
}

void DbModel::batchInsertWords (const std::vector<WordEntry> &wordEntries,
                                std::function<void (int, int)> progressCallback)
{
    if (!isDictDbOpen ())
    {
        return;
    }

    try
    {
        // to be implemented
    }
    catch (const SQLite::Exception &e)
    {
        logErr ("Error batch inserting words", e);
    }
    catch (const std::exception &e)
    {
        logErr ("Unknown error batch inserting words", e);
    }
    catch (...)
    {
        logErr ("Unknown error batch inserting words",
                std::runtime_error ("Unknown exception"));
    }
}
std::optional<WordEntry> DbModel::lookupWord (const QString &word,
                                              const QString &lang)
{
    if (!isDictDbOpen ())
    {
        return std::nullopt;
    }

    return std::nullopt;
}

std::vector<WordEntry> DbModel::searchWords (const QString &pattern,
                                             const QString &lang, int limit)
{
    return std::vector<WordEntry> ();
}

void DbModel::addToUserVocabulary (const QString &userId, const QString &word)
{
    return;
}

void DbModel::removeFromUserVocabulary (const QString &userId,
                                        const QString &word)
{

    return;
}

void updateWordStatus (const QString &userId, const QString &word, int status)
{
    // status: -1= never learned,0=learning, 1=mastered
    return;
}

std::vector<WordEntry> getUserVocabulary (const QString &userId,
                                          int status = -1) // -1 means all
{

    return std::vector<WordEntry> ();
}