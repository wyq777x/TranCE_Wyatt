#pragma once
#include <QString>
#include <SQLiteCpp/SQLiteCpp.h>
#include <Utility/WordEntry.h>
#include <memory>
#include <optional>
#include <qtmetamacros.h>
#include <vector>
class DictDbModel
{
public:
    DictDbModel () = delete;
    explicit DictDbModel (const QString &dbPath);

    DictDbModel (const DictDbModel &) = delete;
    DictDbModel operator= (const DictDbModel &) = delete;

    DictDbModel (DictDbModel &&) = default;
    DictDbModel &operator= (DictDbModel &&) = default;
    ~DictDbModel () = default;

    // Database connection and management
    void openDb ();
    void closeDb ();
    bool isDbOpen () const;

    void initTables ();
    std::optional<WordEntry> lookupWord (const QString &word,
                                         const QString &lang);
    std::vector<WordEntry> searchWords (const QString &pattern,
                                        const QString &lang, int limit = 10);

    bool addToUserVocabulary (const QString &userId, const QString &word,
                              const QString &lang);
    bool removeFromUserVocabulary (const QString &userId, const QString &word);
    bool updateWordStatus (const QString &userId, const QString &word,
                           int status); // 0=learning, 1=mastered

    std::vector<WordEntry> getUserVocabulary (const QString &userId,
                                              int status = -1); // -1 means all

    QString getLastError () const;

private:
    std::unique_ptr<SQLite::Database> m_db;
    QString m_lastError;
    void logErr (const std::string &errMsg, const SQLite::Exception &e) const;
};