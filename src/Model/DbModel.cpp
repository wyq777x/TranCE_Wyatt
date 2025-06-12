#include "DbModel.h"
#include "SQLiteCpp/Exception.h"
#include "SQLiteCpp/Statement.h"
#include "SQLiteCpp/Transaction.h"
#include "Utility/Result.h"
#include <qdebug.h>
#include <qhashfunctions.h>
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

std::optional<QString> DbModel::getUserId (const QString &username) const
{
    if (!isUserDbOpen ())
    {
        return std::nullopt;
    }

    if (username.isEmpty ())
    {
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
        return std::nullopt;
    }

    if (userId.isEmpty ())
    {
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
        return QString ();
    }

    if (username.isEmpty ())
    {
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
                batch.push_back (std::move (entry));
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
                batch.push_back (std::move (entry));
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
        return;

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
        co_return;

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

void DbModel::updateWordStatus (const QString &userId, const QString &word,
                                int status)
{
    // status: -1= never learned,0=learning, 1=mastered
    return;
}

std::vector<WordEntry> DbModel::getUserVocabulary (const QString &userId,
                                                   int status) // -1 means all
{

    return std::vector<WordEntry> ();
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
                qDebug () << "Dictionary file not found at:" << dictFilePath;
            }
        }
        catch (const SQLite::Exception &e)
        {
            logErr ("Error checking/importing dictionary", e);
        }

        m_needsDictImport = false;
    }

    co_return;
}