#pragma once

#include "Utility/AiProviderConfig.h"
#include "Utility/Result.h"
#include <QNetworkAccessManager>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>
#include <functional>
#include <optional>

/**
 * @brief Manages the list of OpenAI-compatible AI providers bound to the
 *        logged-in user profile: database persistence, API key custody via
 *        SecretStore and connectivity testing against the provider's
 *        /models endpoint.
 */
class AiProviderManager : public QObject
{
    Q_OBJECT

public:
    static AiProviderManager &getInstance ()
    {
        static AiProviderManager instance;
        return instance;
    }

    AiProviderManager (const AiProviderManager &) = delete;
    AiProviderManager &operator= (const AiProviderManager &) = delete;
    AiProviderManager (AiProviderManager &&) = delete;
    AiProviderManager &operator= (AiProviderManager &&) = delete;

    QVector<AiProviderConfig> getProviders (const QString &userId) const;

    std::optional<AiProviderConfig>
    getActiveProvider (const QString &userId) const;

    // Persists provider config and stores apiKey (when non-empty) in the
    // SecretStore under a name-derived reference. Passing an empty apiKey
    // on update keeps the previously stored key.
    ChangeResult saveProvider (const QString &userId,
                               AiProviderConfig &provider,
                               const QString &apiKey);

    ChangeResult removeProvider (const QString &userId,
                                 const AiProviderConfig &provider);

    ChangeResult activateProvider (const QString &userId, qlonglong providerId);

    // Resolved provider configuration including the plain API key; used to
    // hand the active provider to the AI sidecar session. The key only ever
    // lives in memory.
    struct ResolvedProvider
    {
        AiProviderConfig config;
        QString apiKey;
    };

    std::optional<ResolvedProvider>
    resolveActiveProvider (const QString &userId) const;

    // Async GET {baseUrl}/models with Bearer auth. On success, models
    // contains the reported model ids (chat-capable ones are not filtered
    // here; callers pick). Runs callback on this object's thread.
    using TestCallback = std::function<void (
        bool ok, const QString &errorMessage, const QStringList &models)>;
    void testConnection (const QString &baseUrl, const QString &apiKey,
                         TestCallback callback);

private:
    explicit AiProviderManager () = default;

    static QString secretKeyFor (const QString &userId, const QString &name);

    QNetworkAccessManager m_nam;
};
