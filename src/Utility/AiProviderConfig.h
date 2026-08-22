#pragma once

#include <QMetaType>
#include <QString>
#include <QVector>
#include <QtGlobal>

/**
 * @brief Configuration of one OpenAI-compatible AI provider, bound to a
 *        user profile. The API key itself never lives here: only a
 *        reference into the SecretStore (see Utility/SecretStore.h).
 */
struct AiProviderConfig
{
    qlonglong id = -1; // database row id, -1 for not-yet-persisted
    QString name;
    QString baseUrl;        // e.g. "https://api.openai.com/v1"
    QString apiKeyRef;      // SecretStore key, empty when no key stored
    QString chatModel;      // e.g. "gpt-4o-mini"
    QString embeddingModel; // optional, e.g. "text-embedding-3-small"
    bool isActive = false;

    bool isValid () const
    {
        return !name.isEmpty () && !baseUrl.isEmpty ()
               && !chatModel.isEmpty ();
    }
};

// NOTE: no space before the parenthesis - function-like macros do not
// expand otherwise and the leftover tokens break every header that
// follows.
Q_DECLARE_METATYPE(AiProviderConfig)
