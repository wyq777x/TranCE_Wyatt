#include "DbModel.h"
#include "SQLiteCpp/Exception.h"
#include "SQLiteCpp/Statement.h"
#include "SQLiteCpp/Transaction.h"
#include "Utility/Result.h"
#include <random>
#include <set>
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
        logErr ("User database is not open",
                std::runtime_error ("Database connection is not established"));
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

bool DbModel::userIdExists (const QString &userId) const
{
    if (!isUserDbOpen ())
    {
        logErr ("User database is not open",
                std::runtime_error ("Database connection is not established"));
        return false;
    }

    try
    {
        SQLite::Statement query (
            *user_db, "SELECT COUNT(*) FROM users WHERE user_id = ?");
        query.bind (1, userId.toStdString ());
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
        logErr ("User database is not open",
                std::runtime_error ("Database connection is not established"));
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

ChangeResult DbModel::updateUserPassword (const QString &username,
                                          const QString &oldPasswordHash,
                                          const QString &newPasswordHash)
{
    if (!isUserDbOpen ())
    {
        return ChangeResult::DatabaseError;
    }

    if (username.isEmpty () || oldPasswordHash.isEmpty () ||
        newPasswordHash.isEmpty ())
    {
        return ChangeResult::NullValue;
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

        if (query.getChanges () == 0)
        {
            return ChangeResult::Password_OldIncorrect;
        }

        if (oldPasswordHash == newPasswordHash)
        {
            return ChangeResult::StillSame;
        }

        return ChangeResult::Success;
    }
    catch (const SQLite::Exception &e)
    {
        logErr ("Error updating user password in database", e);
        return ChangeResult::DatabaseError;
    }
    catch (const std::exception &e)
    {
        logErr ("Unknown error updating user password in database", e);
        return ChangeResult::UnknownError;
    }
    catch (...)
    {
        logErr ("Unknown error updating user password in database",
                std::runtime_error ("Unknown exception"));
        return ChangeResult::UnknownError;
    }
}

ChangeResult DbModel::changeUsername (const QString &oldUsername,
                                      const QString &newUsername)
{
    if (!isUserDbOpen ())
    {
        return ChangeResult::DatabaseError;
    }

    if (oldUsername == newUsername)
    {
        return ChangeResult::StillSame; // No change needed
    }

    if (newUsername.isEmpty () || oldUsername.isEmpty ())
    {
        return ChangeResult::NullValue; // New username cannot be empty
    }

    if (userExists (newUsername))
    {
        return ChangeResult::AlreadyExists; // New username already exists
    }

    try
    {
        SQLite::Statement query (*user_db, "UPDATE users SET username = ? "
                                           "WHERE username = ?");

        query.bind (1, newUsername.toStdString ());
        query.bind (2, oldUsername.toStdString ());
        query.exec ();

        if (query.getChanges () < 0)
        {
            return ChangeResult::DatabaseError;
        }

        return ChangeResult::Success;
    }
    catch (const SQLite::Exception &e)
    {
        logErr ("Error changing username in database", e);
        return ChangeResult::DatabaseError;
    }
    catch (const std::exception &e)
    {
        logErr ("Unknown error changing username in database", e);
        return ChangeResult::UnknownError;
    }
    catch (...)
    {
        logErr ("Unknown error changing username in database",
                std::runtime_error ("Unknown exception"));
        return ChangeResult::UnknownError;
    }
}

ChangeResult DbModel::changeEmail (const QString &username,
                                   const QString &oldEmail,
                                   const QString &newEmail)
{
    if (!isUserDbOpen ())
    {
        return ChangeResult::DatabaseError;
    }

    if (oldEmail == newEmail)
    {
        return ChangeResult::StillSame;
    }

    if (username.isEmpty () || newEmail.isEmpty ())
    {
        return ChangeResult::NullValue;
    }

    try
    {
        SQLite::Statement query (
            *user_db, "UPDATE users SET email = ? WHERE username = ?");
        query.bind (1, newEmail.toStdString ());
        query.bind (2, username.toStdString ());
        query.exec ();

        if (query.getChanges () == 0)
        {
            return ChangeResult::DatabaseError;
        }

        return ChangeResult::Success;
    }
    catch (const SQLite::Exception &e)
    {
        logErr ("Error changing username in database", e);
        return ChangeResult::DatabaseError;
    }
    catch (const std::exception &e)
    {
        logErr ("Unknown error changing username in database", e);
        return ChangeResult::UnknownError;
    }
    catch (...)
    {
        logErr ("Unknown error changing username in database",
                std::runtime_error ("Unknown exception"));
        return ChangeResult::UnknownError;
    }
}

ChangeResult DbModel::changeAvatar (const QString &username,
                                    const QString &avatarPath)
{
    if (!isUserDbOpen ())
    {
        return ChangeResult::DatabaseError;
    }
    if (username.isEmpty () || avatarPath.isEmpty ())
    {
        return ChangeResult::NullValue;
    }
    if (!QFile::exists (avatarPath))
    {
        return ChangeResult::FileNotFound;
    }

    QDir avatarDir (getAvatarDir ());
    if (!avatarDir.exists ())
    {
        if (!avatarDir.mkpath ("."))
        {
            logErr ("Failed to create avatar directory",
                    std::runtime_error ("Directory creation failed"));
            return ChangeResult::DatabaseError;
        }
    }

    try
    {
        SQLite::Statement query (
            *user_db, "UPDATE users SET avatar_path = ? WHERE username = ?");
        query.bind (1, avatarPath.toStdString ());
        query.bind (2, username.toStdString ());
        query.exec ();

        if (query.getChanges () == 0)
        {
            return ChangeResult::DatabaseError;
        }

        return ChangeResult::Success;
    }
    catch (const SQLite::Exception &e)
    {
        logErr ("Error changing user avatar in database", e);
        return ChangeResult::DatabaseError;
    }
    catch (const std::exception &e)
    {
        logErr ("Unknown error changing user avatar in database", e);
        return ChangeResult::UnknownError;
    }
    catch (...)
    {
        logErr ("Unknown error changing user avatar in database",
                std::runtime_error ("Unknown exception"));
        return ChangeResult::UnknownError;
    }
}

ChangeResult DbModel::updateReciteProgress (int current, int total,
                                            const QString &userId)
{
    if (!isUserDbOpen ())
    {
        return ChangeResult::DatabaseError;
    }

    if (current < 0 || total <= 0 || current > total)
    {
        logErr ("Invalid progress values",
                std::runtime_error ("Current or total progress is invalid"));
        return ChangeResult::InvalidInput;
    }

    if (userId.isEmpty ())
    {
        logErr ("User ID is empty", std::runtime_error ("Invalid input"));
        return ChangeResult::NullValue;
    }

    try
    {
        SQLite::Statement query (*user_db,
                                 "INSERT OR REPLACE INTO user_progress "
                                 "(user_id, current_progress, total_words) "
                                 "VALUES (?, ?, ?)");
        query.bind (1, userId.toStdString ());
        query.bind (2, current);
        query.bind (3, total);
        query.exec ();

        if (query.getChanges () == 0 && !userIdExists (userId))
        {
            logErr ("No changes made to user progress",
                    std::runtime_error ("User ID not found or no changes"));
            return ChangeResult::DatabaseError;
        }

        return ChangeResult::Success;
    }
    catch (const SQLite::Exception &e)
    {
        logErr ("Error updating recite progress in database", e);
        return ChangeResult::DatabaseError;
    }
    catch (const std::exception &e)
    {
        logErr ("Unknown error updating recite progress in database", e);
        return ChangeResult::UnknownError;
    }
    catch (...)
    {
        logErr ("Unknown error updating recite progress in database",
                std::runtime_error ("Unknown exception"));
        return ChangeResult::UnknownError;
    }
}

std::optional<QString> DbModel::getUserId (const QString &username) const
{
    if (!isUserDbOpen ())
    {
        logErr ("User database is not open",
                std::runtime_error ("Database connection is not established"));
        return std::nullopt;
    }

    if (username.isEmpty ())
    {
        logErr ("Username is empty", std::runtime_error ("Invalid input"));
        return std::nullopt;
    }

    try
    {
        SQLite::Statement query (
            *user_db, "SELECT user_id FROM users WHERE username = ?");
        query.bind (1, username.toStdString ());

        if (query.executeStep ())
        {
            return QString::fromStdString (query.getColumn (0).getString ());
        }

        return std::nullopt;
    }
    catch (const SQLite::Exception &e)
    {
        logErr ("Error getting user ID from database", e);
        return std::nullopt;
    }
    catch (const std::exception &e)
    {
        logErr ("Unknown error getting user ID from database", e);
        return std::nullopt;
    }
    catch (...)
    {
        logErr ("Unknown error getting user ID from database",
                std::runtime_error ("Unknown exception"));
        return std::nullopt;
    }
}

std::optional<QString> DbModel::getUserName (const QString &userId) const
{
    if (!isUserDbOpen ())
    {
        logErr ("User database is not open",
                std::runtime_error ("Database connection is not established"));
        return std::nullopt;
    }

    if (userId.isEmpty ())
    {
        logErr ("User ID is empty", std::runtime_error ("Invalid input"));
        return std::nullopt;
    }

    try
    {
        SQLite::Statement query (
            *user_db, "SELECT username FROM users WHERE user_id = ?");
        query.bind (1, userId.toStdString ());

        if (query.executeStep ())
        {
            return QString::fromStdString (query.getColumn (0).getString ());
        }

        return std::nullopt;
    }
    catch (const SQLite::Exception &e)
    {
        logErr ("Error getting username from database", e);
        return std::nullopt;
    }
    catch (const std::exception &e)
    {
        logErr ("Unknown error getting username from database", e);
        return std::nullopt;
    }
    catch (...)
    {
        logErr ("Unknown error getting username from database",
                std::runtime_error ("Unknown exception"));
        return std::nullopt;
    }
}

QString DbModel::getUserEmail (const QString &username) const
{
    if (!isUserDbOpen ())
    {
        logErr ("User database is not open",
                std::runtime_error ("Database connection is not established"));
        return QString ();
    }

    if (username.isEmpty ())
    {
        logErr ("Username is empty", std::runtime_error ("Invalid input"));
        return QString ();
    }

    try
    {
        SQLite::Statement query (*user_db,
                                 "SELECT email FROM users WHERE username = ?");
        query.bind (1, username.toStdString ());

        if (query.executeStep ())
        {
            // Check if the email column is NULL
            if (!query.getColumn (0).isNull ())
            {
                return QString::fromStdString (
                    query.getColumn (0).getString ());
            }
        }

        return QString (); // Return empty string if no email found or email is
                           // NULL
    }
    catch (const SQLite::Exception &e)
    {
        logErr ("Error getting user email from database", e);
        return QString ();
    }
    catch (const std::exception &e)
    {
        logErr ("Unknown error getting user email from database", e);
        return QString ();
    }
    catch (...)
    {
        logErr ("Unknown error getting user email from database",
                std::runtime_error ("Unknown exception"));
        return QString ();
    }
}

QString DbModel::getUserAvatarPath (const QString &username) const
{
    if (!isUserDbOpen ())
    {
        logErr ("User database is not open",
                std::runtime_error ("Database connection is not established"));
        return QString (":/image/DefaultUser.png");
    }

    if (username.isEmpty ())
    {
        logErr ("Username is empty", std::runtime_error ("Invalid input"));
        return QString (":/image/DefaultUser.png");
    }

    try
    {
        SQLite::Statement query (
            *user_db, "SELECT avatar_path FROM users WHERE username = ?");
        query.bind (1, username.toStdString ());

        if (query.executeStep ())
        {

            if (!query.getColumn (0).isNull ())
            {
                return QString::fromStdString (
                    query.getColumn (0).getString ());
            }
        }

        return QString (":/image/DefaultUser.png");
    }
    catch (const SQLite::Exception &e)
    {
        logErr ("Error getting user avatar path from database", e);
        return QString (":/image/DefaultUser.png");
    }
    catch (const std::exception &e)
    {
        logErr ("Unknown error getting user avatar path from database", e);
        return QString (":/image/DefaultUser.png");
    }
    catch (...)
    {
        logErr ("Unknown error getting user avatar path from database",
                std::runtime_error ("Unknown exception"));
        return QString (":/image/DefaultUser.png");
    }
}

AsyncTask<void> DbModel::importWordEntry (const WordEntry &wordEntry)
{
    if (!isDictDbOpen ())
    {
        logErr ("Dictionary database is not open",
                std::runtime_error ("Database connection is not established"));
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
            translationsQuery.bind (4, "zh");
            translationsQuery.bind (5, 1.0); // Default confidence score
            translationsQuery.exec ();
        }

        // Insert examples

        if (!wordEntry.examples.isEmpty ())
        {
            SQLite::Statement examplesQuery (*dict_db,
                                             "INSERT INTO examples (word, "
                                             "example_sentence,translation) "
                                             "VALUES (?,?,?)");
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
    if (!isDictDbOpen () || wordEntries.empty ())
    {
        logErr ("Dictionary database is not open or no word entries provided",
                std::runtime_error ("Database connection is not established"));
        co_return;
    }

    try
    {
        SQLite::Transaction transaction (*dict_db);

        const int batchSize = 1000;
        for (size_t i = 0; i < wordEntries.size (); i += batchSize)
        {
            size_t endIdx = std::min (i + batchSize, wordEntries.size ());
            std::vector<WordEntry> batch (wordEntries.begin () + i,
                                          wordEntries.begin () + endIdx);

            co_await insertWordBatch (batch);

            // let go of the coroutine to allow other tasks to run
            if (i % (batchSize * 10) == 0)
            {
                co_await std::suspend_always{};
            }
        }

        transaction.commit ();
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
        logErr ("Dictionary database is not open",
                std::runtime_error ("Database connection is not established"));
        co_return;
    }

    try
    {
        QFile csvFile (filePath);

        if (!csvFile.open (QIODevice::ReadOnly | QIODevice::Text))
        {
            logErr ("Cannot open file",
                    std::runtime_error ("File open failed"));
            co_return;
        }

        QTextStream inputStream (&csvFile);
        inputStream.setEncoding (QStringConverter::Utf8);

        // Skip the format example header line
        QString headerLine = inputStream.readLine ();

        constexpr int batchSize = 1000;
        std::vector<WordEntry> batch;
        batch.reserve (batchSize);

        // progress for callback
        long long totalLines = 0;
        long long processedLines = 0;
        QTextStream countStream (&csvFile);
        countStream.seek (inputStream.pos ());
        while (!countStream.atEnd ())
        {
            countStream.readLine ();
            totalLines++;
        }

        inputStream.seek (0); // Reset to the beginning of the file
        inputStream.readLine ();

        SQLite::Transaction transaction (*dict_db);

        while (!inputStream.atEnd ())
        {
            QString line = inputStream.readLine ().trimmed ();
            if (line.isEmpty ())
                continue;

            WordEntry entry = parseCSVLineToWordEntry (line);
            if (!entry.word.isEmpty ())
            {
                batch.emplace_back (std::move (entry));
            }

            processedLines++;

            if (batch.size () >= batchSize)
            {
                co_await insertWordBatch (batch);
                batch.clear ();

                if (progressCallback)
                {
                    progressCallback (processedLines, totalLines);
                }

                if (processedLines % 100 == 0)
                {
                    co_await std::suspend_always{};
                }
            }
        }

        if (!batch.empty ())
        {
            co_await insertWordBatch (batch);
        }

        transaction.commit ();

        if (progressCallback)
        {
            progressCallback (totalLines, totalLines);
        }

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

void DbModel::importFromFileSync (
    const QString &filePath, std::function<void (int, int)> progressCallback)
{
    if (!isDictDbOpen ())
    {
        logErr ("Dictionary database is not open",
                std::runtime_error ("Database connection is not established"));
        return;
    }

    try
    {
        QFile csvFile (filePath);

        if (!csvFile.open (QIODevice::ReadOnly | QIODevice::Text))
        {
            logErr ("Cannot open file",
                    std::runtime_error ("File open failed"));
            return;
        }

        QTextStream inputStream (&csvFile);
        inputStream.setEncoding (QStringConverter::Utf8);

        // Skip the header line
        QString headerLine = inputStream.readLine ();

        constexpr int batchSize = 1000;
        std::vector<WordEntry> batch;
        batch.reserve (batchSize);

        // Count lines for progress
        long long totalLines = 0;
        long long processedLines = 0;
        QTextStream countStream (&csvFile);
        countStream.seek (inputStream.pos ());
        while (!countStream.atEnd ())
        {
            countStream.readLine ();
            totalLines++;
        }

        inputStream.seek (0);
        inputStream.readLine (); // Skip header again

        SQLite::Transaction transaction (*dict_db);

        while (!inputStream.atEnd ())
        {
            QString line = inputStream.readLine ().trimmed ();
            if (line.isEmpty ())
                continue;

            WordEntry entry = parseCSVLineToWordEntry (line);
            if (!entry.word.isEmpty ())
            {
                batch.emplace_back (std::move (entry));
            }

            processedLines++;

            if (batch.size () >= batchSize)
            {
                insertWordBatchSync (batch);
                batch.clear ();

                if (progressCallback)
                {
                    progressCallback (processedLines, totalLines);
                }
            }
        }

        if (!batch.empty ())
        {
            insertWordBatchSync (batch);
        }

        transaction.commit ();

        if (progressCallback)
        {
            progressCallback (totalLines, totalLines);
        }
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

void DbModel::insertWordBatchSync (const std::vector<WordEntry> &batch)
{
    if (batch.empty ())
    {
        logErr ("Batch is empty", std::runtime_error ("No words to insert"));
        return;
    }

    try
    {
        SQLite::Statement wordsStmt (
            *dict_db, "INSERT OR IGNORE INTO words "
                      "(word, part_of_speech, pronunciation, frequency, notes) "
                      "VALUES (?, ?, ?, ?, ?)");

        SQLite::Statement translationsStmt (
            *dict_db, "INSERT OR IGNORE INTO word_translations "
                      "(source_word, source_language, target_word, "
                      "target_language, confidence_score) "
                      "VALUES (?, ?, ?, ?, ?)");

        for (const auto &entry : batch)
        {

            wordsStmt.reset ();
            wordsStmt.bind (1, entry.word.toStdString ());
            wordsStmt.bind (2, entry.partOfSpeech.toStdString ());
            wordsStmt.bind (3, entry.pronunciation.toStdString ());
            wordsStmt.bind (4, entry.frequency);
            wordsStmt.bind (5, entry.notes.toStdString ());
            wordsStmt.exec ();

            if (!entry.translation.isEmpty ())
            {
                translationsStmt.reset ();
                translationsStmt.bind (1, entry.word.toStdString ());
                translationsStmt.bind (2, entry.language.toStdString ());
                translationsStmt.bind (3, entry.translation.toStdString ());
                translationsStmt.bind (4, "zh");
                translationsStmt.bind (5, 1.0);
                translationsStmt.exec ();
            }
        }
    }
    catch (const SQLite::Exception &e)
    {
        logErr ("Error in batch insert", e);
        throw;
    }
    catch (const std::exception &e)
    {
        logErr ("Unknown error in batch insert", e);
        throw;
    }
    catch (...)
    {
        logErr ("Unknown error in batch insert",
                std::runtime_error ("Unknown exception"));
        throw;
    }
}

AsyncTask<void> DbModel::insertWordBatch (const std::vector<WordEntry> &batch)
{
    if (batch.empty ())
    {
        logErr ("Batch is empty", std::runtime_error ("No words to insert"));
        co_return;
    }

    try
    {

        SQLite::Statement wordsStmt (
            *dict_db, "INSERT OR IGNORE INTO words "
                      "(word, part_of_speech, pronunciation, frequency, notes) "
                      "VALUES (?, ?, ?, ?, ?)");

        SQLite::Statement translationsStmt (
            *dict_db, "INSERT OR IGNORE INTO word_translations "
                      "(source_word, source_language, target_word, "
                      "target_language, confidence_score) "
                      "VALUES (?, ?, ?, ?, ?)");

        for (const auto &entry : batch)
        {

            wordsStmt.reset ();
            wordsStmt.bind (1, entry.word.toStdString ());
            wordsStmt.bind (2, entry.partOfSpeech.toStdString ());
            wordsStmt.bind (3, entry.pronunciation.toStdString ());
            wordsStmt.bind (4, entry.frequency);
            wordsStmt.bind (5, entry.notes.toStdString ());
            wordsStmt.exec ();

            if (!entry.translation.isEmpty ())
            {
                translationsStmt.reset ();
                translationsStmt.bind (1, entry.word.toStdString ());
                translationsStmt.bind (2, entry.language.toStdString ());
                translationsStmt.bind (3, entry.translation.toStdString ());
                translationsStmt.bind (4, "zh");
                translationsStmt.bind (5, 1.0);
                translationsStmt.exec ();
            }
        }
    }
    catch (const SQLite::Exception &e)
    {
        logErr ("Error in batch insert", e);
        throw;
    }
    catch (const std::exception &e)
    {
        logErr ("Unknown error in batch insert", e);
        throw;
    }
    catch (...)
    {
        logErr ("Unknown error in batch insert",
                std::runtime_error ("Unknown exception"));
        throw;
    }

    co_return;
}

std::optional<WordEntry> DbModel::lookupWord (const QString &word,
                                              const QString &srcLang)
{
    if (!isDictDbOpen ())
    {
        logErr ("Dictionary database is not open",
                std::runtime_error ("Database connection is not established"));
        return std::nullopt;
    }

    if (word.isEmpty () || srcLang.isEmpty ())
    {
        logErr ("Word or source language is empty",
                std::runtime_error ("Invalid input"));
        return std::nullopt; // Invalid input
    }
    if (srcLang != "en" && srcLang != "zh")
    {
        logErr ("Unsupported source language", std::runtime_error ("Invalid "
                                                                   "source "
                                                                   "language"));
        return std::nullopt; // Unsupported language
    }
    try
    {
        WordEntry entry;
        if (srcLang == "en")
        {
            SQLite::Statement queryWordBasic (*dict_db,
                                              "SELECT word, pronunciation "
                                              "FROM words WHERE word = ?");
            queryWordBasic.bind (1, word.toStdString ());
            if (queryWordBasic.executeStep ())
            {

                entry.word = QString::fromStdString (
                    queryWordBasic.getColumn (0).getString ());
                entry.pronunciation = QString::fromStdString (
                    queryWordBasic.getColumn (1).getString ());
                entry.language = srcLang;
            }

            SQLite::Statement queryTranslation (
                *dict_db, "SELECT target_word, target_language "
                          "FROM word_translations WHERE source_word = ? "
                          "AND source_language = ?");
            queryTranslation.bind (1, word.toStdString ());
            queryTranslation.bind (2, srcLang.toStdString ());
            if (queryTranslation.executeStep ())
            {
                entry.translation = QString::fromStdString (
                    queryTranslation.getColumn (0).getString ());
            }
            else
            {
                entry.translation = ""; // No translation found
            }
        }

        else if (srcLang == "zh")
        {
            SQLite::Statement queryWord (
                *dict_db, "SELECT source_word FROM word_translations WHERE "
                          "target_word = ? AND target_language = ?");
            queryWord.bind (1, word.toStdString ());
            queryWord.bind (2, srcLang.toStdString ());
            if (queryWord.executeStep ())
            {
                entry.translation = word;
                entry.word = QString::fromStdString (
                    queryWord.getColumn (0).getString ());
                entry.language = srcLang;

                // get pronunciation
                SQLite::Statement queryPronunciation (
                    *dict_db, "SELECT pronunciation FROM words WHERE word = ?");
                queryPronunciation.bind (1, entry.word.toStdString ());
                if (queryPronunciation.executeStep ())
                {
                    entry.pronunciation = QString::fromStdString (
                        queryPronunciation.getColumn (0).getString ());
                }
                else
                {
                    entry.pronunciation = ""; // No pronunciation found
                }
            }
        }

        return entry;
    }
    catch (SQLite::Exception &e)
    {
        logErr ("Error looking up word in database", e);
        return std::nullopt;
    }
    catch (const std::exception &e)
    {
        logErr ("Unknown error looking up word in database", e);
        return std::nullopt;
    }
    catch (...)
    {
        logErr ("Unknown error looking up word in database",
                std::runtime_error ("Unknown exception"));
        return std::nullopt;
    }
    return std::nullopt;
}

std::vector<WordEntry> DbModel::searchWords (const QString &pattern,
                                             const QString &srcLang, int limit)
{
    if (!isDictDbOpen ())
    {
        logErr ("Dictionary database is not open",
                std::runtime_error ("Database connection is not established"));
        return std::vector<WordEntry> ();
    }
    if (pattern.isEmpty () || srcLang.isEmpty ())
    {
        logErr ("Pattern or source language is empty",
                std::runtime_error ("Invalid input"));
        return std::vector<WordEntry> (); // Invalid input
    }

    if (srcLang != "en" && srcLang != "zh")
    {
        logErr ("Unsupported source language", std::runtime_error ("Invalid "
                                                                   "source "
                                                                   "language"));
        return std::vector<WordEntry> (); // Unsupported language
    }

    if (limit <= 0)
    {
        logErr ("Limit must be greater than 0",
                std::runtime_error ("Invalid limit value"));
        return std::vector<WordEntry> (); // Invalid limit
    }

    try
    {

        std::vector<WordEntry> results;
        if (srcLang == "en")
        {
            SQLite::Statement queryWordsBasic (
                *dict_db,
                "SELECT word,pronunciation ,"
                "CASE"
                " WHEN word = ? THEN 1 "    // precise match
                " WHEN word LIKE ? THEN 2 " // prefix match
                " WHEN word LIKE ? THEN 3 " // include match
                " ELSE 4 "
                " END AS match_priority "
                " FROM words "
                " WHERE word = ? OR "
                " word LIKE ? OR "
                " word LIKE ? "
                " ORDER BY match_priority, word "
                " LIMIT ? ; ");

            queryWordsBasic.bind (1, pattern.toStdString ());
            queryWordsBasic.bind (2, pattern.toStdString () + "%");
            queryWordsBasic.bind (3, "%" + pattern.toStdString () + "%");
            queryWordsBasic.bind (4, pattern.toStdString ());
            queryWordsBasic.bind (5, pattern.toStdString () + "%");
            queryWordsBasic.bind (6, "%" + pattern.toStdString () + "%");
            queryWordsBasic.bind (7, limit);

            while (queryWordsBasic.executeStep ())
            {
                WordEntry entry;
                entry.word = QString::fromStdString (
                    queryWordsBasic.getColumn (0).getString ());
                entry.pronunciation = QString::fromStdString (
                    queryWordsBasic.getColumn (1).getString ());
                entry.language = srcLang;

                // Fetch translation
                SQLite::Statement queryTranslation (
                    *dict_db, "SELECT target_word, target_language "
                              "FROM word_translations WHERE source_word = ? "
                              "AND source_language = ?");
                queryTranslation.bind (1, entry.word.toStdString ());
                queryTranslation.bind (2, srcLang.toStdString ());

                if (queryTranslation.executeStep ())
                {
                    entry.translation = QString::fromStdString (
                        queryTranslation.getColumn (0).getString ());
                }
                else
                {
                    entry.translation = ""; // No translation found
                }

                results.emplace_back (entry);
            }
        }
        else if (srcLang == "zh")
        {
            SQLite::Statement queryTranslations (
                *dict_db,
                "SELECT source_word, target_word ,"
                "CASE"
                " WHEN target_word = ? THEN 1 " // precise match
                " WHEN target_word LIKE '_. %' AND target_word LIKE ? THEN "
                "2 " // single letter+dot+space prefix (e.g., "a. xxx")
                " WHEN target_word LIKE '__. %' AND target_word LIKE ? "
                "THEN 3 " // two letters+dot+space prefix (e.g., "ad. xxx")
                " WHEN target_word LIKE '___. %' AND target_word LIKE ? "
                "THEN 4 " // three letters+dot+space prefix (e.g., "adj. xxx")
                " WHEN target_word LIKE '. %' AND target_word LIKE ? THEN "
                "5 " // dot+space prefix (e.g., ". xxx")
                " WHEN target_word LIKE ? THEN 6 " // normal prefix match
                " WHEN target_word LIKE ? THEN 7 " // include match
                " ELSE 8 "
                " END AS match_priority "
                " FROM word_translations "
                " WHERE target_language = 'zh' AND ("
                " target_word = ? OR "
                " (target_word LIKE '_. %' AND target_word LIKE ?) OR "
                " (target_word LIKE '__. %' AND target_word LIKE ?) OR "
                " (target_word LIKE '___. %' AND target_word LIKE ?) OR "
                " (target_word LIKE '. %' AND target_word LIKE ?) OR "
                " target_word LIKE ? OR "
                " target_word LIKE ? )"
                " ORDER BY match_priority, target_word "
                " LIMIT ? ; ");

            queryTranslations.bind (1, pattern.toStdString ());
            queryTranslations.bind (2, "_. " + pattern.toStdString () + "%");
            queryTranslations.bind (3, "__. " + pattern.toStdString () + "%");
            queryTranslations.bind (4, "___. " + pattern.toStdString () + "%");
            queryTranslations.bind (5, ". " + pattern.toStdString () + "%");
            queryTranslations.bind (6, pattern.toStdString () + "%");
            queryTranslations.bind (7, "%" + pattern.toStdString () + "%");
            queryTranslations.bind (8, pattern.toStdString ());
            queryTranslations.bind (9, "_. " + pattern.toStdString () + "%");
            queryTranslations.bind (10, "__. " + pattern.toStdString () + "%");
            queryTranslations.bind (11, "___. " + pattern.toStdString () + "%");
            queryTranslations.bind (12, ". " + pattern.toStdString () + "%");
            queryTranslations.bind (13, pattern.toStdString () + "%");
            queryTranslations.bind (14, "%" + pattern.toStdString () + "%");
            queryTranslations.bind (15, limit);

            while (queryTranslations.executeStep ())
            {
                WordEntry entry;
                entry.word = QString::fromStdString (
                    queryTranslations.getColumn (0).getString ());
                entry.translation = QString::fromStdString (
                    queryTranslations.getColumn (1).getString ());
                entry.language = srcLang;

                // Fetch pronunciation for the English word
                SQLite::Statement queryPronunciation (
                    *dict_db, "SELECT pronunciation FROM words WHERE word = ?");
                queryPronunciation.bind (1, entry.word.toStdString ());

                if (queryPronunciation.executeStep ())
                {
                    entry.pronunciation = QString::fromStdString (
                        queryPronunciation.getColumn (0).getString ());
                }
                else
                {
                    entry.pronunciation = ""; // No pronunciation found
                }

                results.emplace_back (entry);
            }
        }

        return results;
    }
    catch (SQLite::Exception &e)
    {
        logErr ("Error searching words in database", e);
        return std::vector<WordEntry> ();
    }
    catch (const std::exception &e)
    {
        logErr ("Unknown error searching words in database", e);
        return std::vector<WordEntry> ();
    }
    catch (...)
    {
        logErr ("Unknown error searching words in database",
                std::runtime_error ("Unknown exception"));
        return std::vector<WordEntry> ();
    }
    return std::vector<WordEntry> ();
}

void DbModel::addToUserFavorites (const QString &userId, const QString &word)
{
    if (!isUserDbOpen ())
    {
        logErr ("User database is not open",
                std::runtime_error ("Database connection is not established"));
        return;
    }

    if (isWordFavorited (userId, word))
    {
        return;
    }

    try
    {
        SQLite::Statement query (
            *user_db,
            "INSERT INTO user_favorites (user_id, word) VALUES (?, ?)");
        query.bind (1, userId.toStdString ());
        query.bind (2, word.toStdString ());
        query.exec ();
    }
    catch (const SQLite::Exception &e)
    {
        logErr ("Error adding word to favorites", e);
    }
    catch (const std::exception &e)
    {
        logErr ("Unknown error adding word to favorites", e);
    }
    catch (...)
    {
        logErr ("Unknown error adding word to favorites",
                std::runtime_error ("Unknown exception"));
    }
}

void DbModel::removeFromUserFavorites (const QString &userId,
                                       const QString &word)
{
    if (!isUserDbOpen ())
    {
        logErr ("User database is not open",
                std::runtime_error ("Database connection is not established"));
        return;
    }

    try
    {
        SQLite::Statement query (
            *user_db,
            "DELETE FROM user_favorites WHERE user_id = ? AND word = ?");
        query.bind (1, userId.toStdString ());
        query.bind (2, word.toStdString ());
        query.exec ();
    }
    catch (const SQLite::Exception &e)
    {
        logErr ("Error removing word from favorites", e);
    }
    catch (const std::exception &e)
    {
        logErr ("Unknown error removing word from favorites", e);
    }
    catch (...)
    {
        logErr ("Unknown error removing word from favorites",
                std::runtime_error ("Unknown exception"));
    }
}

bool DbModel::isWordFavorited (const QString &userId, const QString &word) const
{
    if (!isUserDbOpen ())
    {
        logErr ("User database is not open",
                std::runtime_error ("Database connection is not established"));
        return false;
    }

    try
    {
        SQLite::Statement query (*user_db,
                                 "SELECT COUNT(*) FROM user_favorites WHERE "
                                 "user_id = ? AND word = ?");
        query.bind (1, userId.toStdString ());
        query.bind (2, word.toStdString ());
        query.executeStep ();
        return query.getColumn (0).getInt () > 0;
    }
    catch (const SQLite::Exception &e)
    {
        logErr ("Error checking if word is favorited", e);
        return false;
    }
    catch (const std::exception &e)
    {
        logErr ("Unknown error checking if word is favorited", e);
        return false;
    }
    catch (...)
    {
        logErr ("Unknown error checking if word is favorited",
                std::runtime_error ("Unknown exception"));
        return false;
    }
}

ChangeResult DbModel::updateWordStatus (const QString &userId,
                                        const QString &word, int status)
{
    // Building...
    // status: 0=learning, 1=mastered

    if (!isUserDbOpen ())
    {
        logErr ("User database is not open",
                std::runtime_error ("Database connection is not established"));
        return ChangeResult::DatabaseError;
    }

    if (userId.isEmpty () || word.isEmpty ())
    {
        logErr ("User ID or word is empty",
                std::runtime_error ("Invalid input"));
        return ChangeResult::InvalidInput;
    }

    if (status < 0 || status > 1)
    {
        logErr ("Invalid status value",
                std::runtime_error ("Status must be 0 or 1"));
        return ChangeResult::InvalidInput;
    }
    try
    {
        SQLite::Statement query (
            *user_db,
            "INSERT OR REPLACE INTO "
            "user_vocabulary (user_id, word, status) VALUES (?, ?, ?)");
        query.bind (1, userId.toStdString ());
        query.bind (2, word.toStdString ());
        query.bind (3, status);
        query.exec ();

        if (query.getChanges () == 0)
        {
            logErr ("No changes made to user vocabulary",
                    std::runtime_error ("Word already has the same status"));
            return ChangeResult::StillSame;
        }

        return ChangeResult::Success;
    }
    catch (const SQLite::Exception &e)
    {

        logErr ("Error updating word status in database", e);
        return ChangeResult::DatabaseError;
    }
    catch (const std::exception &e)
    {
        logErr ("Unknown error updating word status in database", e);
        return ChangeResult::UnknownError;
    }
    catch (...)
    {
        logErr ("Unknown error updating word status in database",
                std::runtime_error ("Unknown exception"));
        return ChangeResult::UnknownError;
    }
}

std::vector<QString> DbModel::getUserVocabulary (const QString &userId,
                                                 int status) // -1 means all
{

    if (!isUserDbOpen ())
    {
        logErr ("User database is not open",
                std::runtime_error ("Database connection is not established"));
        return std::vector<QString> ();
    }

    if (userId.isEmpty ())
    {
        logErr ("User ID is empty", std::runtime_error ("Invalid input"));
        return std::vector<QString> (); // Invalid input
    }

    if (status < -1 || status > 1)
    {
        logErr ("Invalid status value",
                std::runtime_error ("Status must be -1, 0 or 1"));
        return std::vector<QString> (); // Invalid status
    }

    try
    {
        std::vector<QString> vocabularies;

        if (status == -1) // Get all
        {
            SQLite::Statement query (
                *user_db,
                "SELECT word, status FROM user_vocabulary WHERE user_id = ?");
            query.bind (1, userId.toStdString ());

            while (query.executeStep ())
            {
                QString word =
                    QString::fromStdString (query.getColumn (0).getString ());
                int wordStatus = query.getColumn (1).getInt ();

                if (wordStatus == 0 ||
                    wordStatus == 1) // Only learning or mastered
                {
                    vocabularies.emplace_back (word);
                }
            }
            return vocabularies;
        }
        else
        {
            SQLite::Statement query (
                *user_db,
                "SELECT word FROM user_vocabulary WHERE user_id = ? AND "
                "status = ?");
            query.bind (1, userId.toStdString ());
            query.bind (2, status);
            while (query.executeStep ())
            {
                QString word =
                    QString::fromStdString (query.getColumn (0).getString ());
                vocabularies.emplace_back (word);
            }
            return vocabularies;
        }
    }
    catch (const SQLite::Exception &e)
    {
        logErr ("Error getting user vocabulary from database", e);
        return std::vector<QString> ();
    }
    catch (const std::exception &e)
    {
        logErr ("Unknown error getting user vocabulary from database", e);
        return std::vector<QString> ();
    }
    catch (...)
    {
        logErr ("Unknown error getting user vocabulary from database",
                std::runtime_error ("Unknown exception"));
        return std::vector<QString> ();
    }
}

std::optional<WordEntry> DbModel::getRandomWord ()
{
    if (!isDictDbOpen ())
    {
        logErr ("Dictionary database is not open",
                std::runtime_error ("Database connection is not established"));
        return std::nullopt;
    }

    try
    {
        WordEntry randomWordEntry;

        SQLite::Statement queryRandomWord (
            *dict_db, "SELECT word, pronunciation "
                      "FROM words ORDER BY RANDOM() LIMIT 1");

        if (queryRandomWord.executeStep ())
        {
            randomWordEntry.word = QString::fromStdString (
                queryRandomWord.getColumn (0).getString ());
            randomWordEntry.pronunciation = QString::fromStdString (
                queryRandomWord.getColumn (1).getString ());
            randomWordEntry.language = "en"; // Default language
        }

        SQLite::Statement queryTranslation (
            *dict_db, "SELECT target_word, target_language "
                      "FROM word_translations WHERE source_word = ? "
                      "AND source_language = ?");

        queryTranslation.bind (1, randomWordEntry.word.toStdString ());
        queryTranslation.bind (2, "en"); // Default source language

        if (queryTranslation.executeStep ())
        {
            randomWordEntry.translation = QString::fromStdString (
                queryTranslation.getColumn (0).getString ());
        }
        else
        {
            randomWordEntry.translation = ""; // No translation found
        }

        return randomWordEntry;
    }
    catch (SQLite::Exception &e)
    {
        logErr ("Error getting random word from database", e);
        return std::nullopt;
    }
    catch (const std::exception &e)
    {
        logErr ("Unknown error getting random word from database", e);
        return std::nullopt;
    }
    catch (...)
    {
        logErr ("Unknown error getting random word from database",
                std::runtime_error ("Unknown exception"));
        return std::nullopt;
    }
}

WordEntry DbModel::parseCSVLineToWordEntry (const QString &csvLine)
{
    WordEntry entry;
    QStringList fields = parseCSVLine (csvLine);
    if (fields.size () < 4)
    {
        return entry; // Need at least word, phonetic, definition,
                      // translation
    }

    // CSV format:
    // word,phonetic,definition,translation,pos,collins,oxford,tag,bnc,frq,exchange,detail,audio
    entry.word = fields[0].trimmed ();
    entry.pronunciation = fields.size () > 1 ? fields[1].trimmed () : "";
    entry.translation = fields.size () > 3 ? fields[3].trimmed () : "";
    entry.partOfSpeech = fields.size () > 4 ? fields[4].trimmed () : "";
    entry.language = "en";

    // frq is at index 9
    if (fields.size () > 9)
    {
        bool ok;
        int freq = fields[9].toInt (&ok);
        entry.frequency = ok ? freq : 0;
    }
    else
    {
        entry.frequency = 0;
    }

    return entry;
}

QStringList DbModel::parseCSVLine (const QString &line)
{
    QStringList result;
    QString current;
    bool inQuotes = false;

    for (int i = 0; i < line.length (); ++i)
    {
        QChar c = line[i];

        if (c == '"')
        {
            if (inQuotes && i + 1 < line.length () && line[i + 1] == '"')
            {
                current += '"';
                ++i;
            }
            else
            {
                inQuotes = !inQuotes;
            }
        }
        else if (c == ',' && !inQuotes)
        {
            result << current;
            current.clear ();
        }
        else
        {
            current += c;
        }
    }

    result << current;
    return result;
}

AsyncTask<void> DbModel::initializeAsync (const QString &dictPath)
{
    initDBs ();

    if (m_needsDictImport && isDictDbOpen ())
    {

        QString dictFilePath = dictPath;
        if (dictFilePath.isEmpty ())
        {
            // Default CSV file path
            dictFilePath = QCoreApplication::applicationDirPath () +
                           "/../3rdpart/ECDICT/ecdict.csv";
        }

        try
        {
            SQLite::Statement query (*dict_db, "SELECT COUNT(*) FROM words");
            query.executeStep ();
            int wordCount = query.getColumn (0).getInt ();

            if (wordCount == 0 && QFile::exists (dictFilePath))
            {
                qDebug () << "Dictionary is empty, importing from:"
                          << dictFilePath;

                // Use synchronous import to ensure it works reliably
                importFromFileSync (
                    dictFilePath,
                    [] (int current, int total)
                    {
                        if (current % 5000 == 0 || current == total)
                        {
                            qDebug () << "Dictionary import progress:"
                                      << (current * 100 / total) << "%";
                        }
                    });
                qDebug () << "Dictionary import completed!";
            }
            else if (wordCount > 0)
            {
                qDebug () << "Dictionary already contains" << wordCount
                          << "words";
            }
            else if (!QFile::exists (dictFilePath))
            {
                auto exception =
                    std::runtime_error ("Dictionary file not found at: " +
                                        dictFilePath.toStdString ());
                logErr ("Dictionary file not found", exception);
            }
        }
        catch (const SQLite::Exception &e)
        {
            logErr ("Error checking/importing dictionary", e);
        }
        catch (const std::exception &e)
        {
            logErr ("Unknown error checking/importing dictionary", e);
        }
        catch (...)
        {
            logErr ("Unknown error checking/importing dictionary",
                    std::runtime_error ("Unknown exception"));
        }

        m_needsDictImport = false;
    }

    co_return;
}

void DbModel::addToSearchHistory (const QString &userId, const QString &word)
{
    if (!isUserDbOpen ())
    {
        logErr ("User database is not open",
                std::runtime_error ("Database connection is not established"));
        return;
    }

    try
    {
        SQLite::Statement query (*user_db,
                                 "INSERT INTO user_search_history (user_id, "
                                 "search_word) VALUES (?, ?)");
        query.bind (1, userId.toStdString ());
        query.bind (2, word.toStdString ());
        query.exec ();
    }
    catch (const SQLite::Exception &e)
    {
        logErr ("Error adding to search history", e);
    }
    catch (const std::exception &e)
    {
        logErr ("Unknown error adding to search history", e);
    }
    catch (...)
    {
        logErr ("Unknown error adding to search history",
                std::runtime_error ("Unknown exception"));
    }
}

void DbModel::removeFromSearchHistory (const QString &userId,
                                       const QString &word)
{
    if (!isUserDbOpen ())
    {
        logErr ("User database is not open",
                std::runtime_error ("Database connection is not established"));
        return;
    }

    try
    {
        SQLite::Statement query (
            *user_db, "DELETE FROM user_search_history WHERE user_id = ? "
                      "AND search_word = ?");
        query.bind (1, userId.toStdString ());
        query.bind (2, word.toStdString ());
        query.exec ();
    }
    catch (const SQLite::Exception &e)
    {
        logErr ("Error removing from search history", e);
    }
    catch (const std::exception &e)
    {
        logErr ("Unknown error removing from search history", e);
    }
    catch (...)
    {
        logErr ("Unknown error removing from search history",
                std::runtime_error ("Unknown exception"));
    }
}

void DbModel::addToReciteHistory (const QString &userId, const QString &word)
{
    if (!isUserDbOpen ())
    {
        logErr ("User database is not open",
                std::runtime_error ("Database connection is not established"));
        return;
    }

    try
    {
        SQLite::Statement query (
            *user_db, "INSERT INTO user_recite_history (user_id, recite_word) "
                      "VALUES (?, ?)");
        query.bind (1, userId.toStdString ());
        query.bind (2, word.toStdString ());
        query.exec ();
    }
    catch (const SQLite::Exception &e)
    {
        logErr ("Error adding to recite history", e);
    }
    catch (const std::exception &e)
    {
        logErr ("Unknown error adding to recite history", e);
    }
    catch (...)
    {
        logErr ("Unknown error adding to recite history",
                std::runtime_error ("Unknown exception"));
    }
}

void DbModel::removeFromReciteHistory (const QString &userId,
                                       const QString &word)
{
    if (!isUserDbOpen ())
    {
        logErr ("User database is not open",
                std::runtime_error ("Database connection is not established"));
        return;
    }

    try
    {
        SQLite::Statement query (
            *user_db, "DELETE FROM user_recite_history WHERE user_id = ? "
                      "AND recite_word = ?");
        query.bind (1, userId.toStdString ());
        query.bind (2, word.toStdString ());
        query.exec ();
    }
    catch (const SQLite::Exception &e)
    {
        logErr ("Error removing from recite history", e);
    }
    catch (const std::exception &e)
    {
        logErr ("Unknown error removing from recite history", e);
    }
    catch (...)
    {
        logErr ("Unknown error removing from recite history",
                std::runtime_error ("Unknown exception"));
    }
}

std::vector<QString> DbModel::getUserSearchHistory (const QString &userId)
{
    std::vector<QString> history;

    if (!isUserDbOpen ())
    {
        logErr ("User database is not open",
                std::runtime_error ("Database connection is not established"));
        return history;
    }

    try
    {
        SQLite::Statement query (
            *user_db, "SELECT DISTINCT search_word FROM user_search_history "
                      "WHERE user_id = ? ORDER BY search_history_id DESC");
        query.bind (1, userId.toStdString ());

        while (query.executeStep ())
        {
            history.emplace_back (
                QString::fromStdString (query.getColumn (0).getString ()));
        }
    }
    catch (const SQLite::Exception &e)
    {
        logErr ("Error getting user search history", e);
    }
    catch (const std::exception &e)
    {
        logErr ("Unknown error getting user search history", e);
    }
    catch (...)
    {
        logErr ("Unknown error getting user search history",
                std::runtime_error ("Unknown exception"));
    }

    return history;
}

std::vector<QString> DbModel::getUserReciteHistory (const QString &userId)
{
    std::vector<QString> history;

    if (!isUserDbOpen ())
    {
        logErr ("User database is not open",
                std::runtime_error ("Database connection is not established"));
        return history;
    }

    try
    {
        SQLite::Statement query (
            *user_db, "SELECT DISTINCT recite_word FROM user_recite_history "
                      "WHERE user_id = ? ORDER BY recite_history_id DESC");
        query.bind (1, userId.toStdString ());

        while (query.executeStep ())
        {
            history.emplace_back (
                QString::fromStdString (query.getColumn (0).getString ()));
        }
    }
    catch (const SQLite::Exception &e)
    {
        logErr ("Error getting user recite history", e);
    }
    catch (const std::exception &e)
    {
        logErr ("Unknown error getting user recite history", e);
    }
    catch (...)
    {
        logErr ("Unknown error getting user recite history",
                std::runtime_error ("Unknown exception"));
    }

    return history;
}

std::pair<int, int> DbModel::getProgress (const QString &userId) const
{

    int currentProgress = 0;
    int totalWords = 15;

    if (!isUserDbOpen ())
    {
        logErr ("User database is not open",
                std::runtime_error ("Database connection is not established"));
        return {0, 15};
    }

    try
    {
        SQLite::Statement query (
            *user_db, "SELECT current_progress,total_words FROM user_progress "
                      "WHERE user_id = ?");

        query.bind (1, userId.toStdString ());

        if (query.executeStep ())
        {
            currentProgress = query.getColumn (0).getInt ();
            totalWords = query.getColumn (1).getInt ();

            return {currentProgress, totalWords};
        }

        return {currentProgress, totalWords};
    }
    catch (const SQLite::Exception &e)
    {

        logErr ("Error getting user progress", e);
        return {currentProgress, totalWords};
    }
    catch (const std::exception &e)
    {
        logErr ("Unknown error getting user progress", e);
        return {currentProgress, totalWords};
    }
    catch (...)
    {
        logErr ("Unknown error getting user progress",
                std::runtime_error ("Unknown exception"));
        return {currentProgress, totalWords};
    }
}

std::vector<QString>
DbModel::getRandomWrongTranslations (const QString &correctTranslation,
                                     int limit)
{
    std::vector<QString> wrongTranslations;
    if (!isDictDbOpen ())
    {
        logErr ("Dictionary database is not open",
                std::runtime_error ("Database connection is not established"));
        return std::vector<QString> ();
    }

    if (correctTranslation.isEmpty () || limit <= 0)
    {
        logErr ("Invalid input for getting wrong translations",
                std::runtime_error ("Correct translation is empty or limit "
                                    "is not positive"));
        return std::vector<QString> ();
    }
    try
    {
        SQLite::Statement query (
            *dict_db, "SELECT MIN(translation_id), MAX(translation_id) FROM "
                      "word_translations; ");
        query.executeStep ();
        int minId = query.getColumn (0).getInt ();
        int maxId = query.getColumn (1).getInt ();

        if (minId == maxId)
        {
            logErr ("No translations found in the database",
                    std::runtime_error ("No translations available"));
            return std::vector<QString> ();
        }

        // Generate random IDs excluding the correct translation

        std::set<int> usedIds;
        std::random_device rd;
        std::mt19937 gen (rd ());
        std::uniform_int_distribution<> dis (minId, maxId);

        while (wrongTranslations.size () < limit)
        {
            int randomId = dis (gen);
            if (usedIds.count (randomId) > 0)
            {
                continue; // Already used this ID
            }
            usedIds.insert (randomId);

            SQLite::Statement translationQuery (
                *dict_db, "SELECT target_word FROM word_translations "
                          "WHERE translation_id = ? AND target_word != ?");
            translationQuery.bind (1, randomId);
            translationQuery.bind (2, correctTranslation.toStdString ());

            if (translationQuery.executeStep ())
            {
                QString translation = QString::fromStdString (
                    translationQuery.getColumn (0).getString ());
                if (!translation.isEmpty ())
                {
                    wrongTranslations.emplace_back (translation);
                }
            }
            if (wrongTranslations.size () >= limit)
            {
                break;
            }
        }

        if (wrongTranslations.empty ())
        {
            logErr ("No wrong translations found",
                    std::runtime_error ("No valid translations available"));
            return std::vector<QString> ();
        }

        return wrongTranslations;
    }
    catch (const SQLite::Exception &e)
    {
        logErr ("Error preparing query for wrong translations", e);
        return std::vector<QString> ();
    }
    catch (const std::exception &e)
    {
        logErr ("Unknown error preparing query for wrong translations", e);
        return std::vector<QString> ();
    }
    catch (...)
    {
        logErr ("Unknown error preparing query for wrong translations",
                std::runtime_error ("Unknown exception"));
        return std::vector<QString> ();
    }
}