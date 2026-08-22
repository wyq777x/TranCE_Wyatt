#include "Controller/AiProviderManager.h"

#include "Controller/DbManager.h"
#include "Model/DbModel.h"
#include "Utility/SecretStore.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

QVector<AiProviderConfig>
AiProviderManager::getProviders (const QString &userId) const
{
    const auto providers = DbManager::getInstance ().getAiProviders (userId);

    return QVector<AiProviderConfig> (providers.begin (), providers.end ());
}

std::optional<AiProviderConfig>
AiProviderManager::getActiveProvider (const QString &userId) const
{
    return DbManager::getInstance ().getActiveAiProvider (userId);
}

QString AiProviderManager::secretKeyFor (const QString &userId,
                                         const QString &name)
{
    return "ai-provider/" + userId + "/" + name;
}

ChangeResult AiProviderManager::saveProvider (const QString &userId,
                                              AiProviderConfig &provider,
                                              const QString &apiKey)
{
    // The SecretStore reference is derived from the provider name, so a
    // rename requires migrating the secret to the new reference.
    const QString oldRef = provider.apiKeyRef;
    const QString newRef = secretKeyFor (userId, provider.name);

    // On update with an empty key input, carry the stored key over so the
    // secret survives name changes without re-entering it.
    const QString effectiveKey =
        !apiKey.isEmpty ()
            ? apiKey
            : (oldRef == newRef ? SecretStore::loadSecret (oldRef)
                                : QString ());

    if (!effectiveKey.isEmpty ()
        && !SecretStore::storeSecret (newRef, effectiveKey))
    {
        return ChangeResult::UnknownError;
    }

    provider.apiKeyRef = newRef;

    const ChangeResult result =
        DbManager::getInstance ().upsertAiProvider (userId, provider);

    if (result != ChangeResult::Success)
    {
        return result;
    }

    if (!oldRef.isEmpty () && oldRef != newRef)
    {
        // The old reference is only dropped after the row update succeeded.
        SecretStore::deleteSecret (oldRef);
    }

    return ChangeResult::Success;
}

ChangeResult AiProviderManager::removeProvider (const QString &userId,
                                                const AiProviderConfig &provider)
{
    const ChangeResult result = DbManager::getInstance ().deleteAiProvider (
        userId, provider.id);

    if (result != ChangeResult::Success)
    {
        return result;
    }

    SecretStore::deleteSecret (provider.apiKeyRef);
    return ChangeResult::Success;
}

ChangeResult AiProviderManager::activateProvider (const QString &userId,
                                                  qlonglong providerId)
{
    return DbManager::getInstance ().setActiveAiProvider (userId, providerId);
}

std::optional<AiProviderManager::ResolvedProvider>
AiProviderManager::resolveActiveProvider (const QString &userId) const
{
    auto provider = getActiveProvider (userId);

    if (!provider.has_value ())
    {
        return std::nullopt;
    }

    ResolvedProvider resolved;
    resolved.config = *provider;

    if (!provider->apiKeyRef.isEmpty ())
    {
        resolved.apiKey = SecretStore::loadSecret (provider->apiKeyRef);
    }

    return resolved;
}

void AiProviderManager::testConnection (const QString &baseUrl,
                                        const QString &apiKey,
                                        TestCallback callback)
{
    QString normalized = baseUrl;

    while (normalized.endsWith ("/"))
    {
        normalized.chop (1);
    }

    const QUrl url (normalized + "/models");
    QNetworkRequest request (url);
    request.setHeader (QNetworkRequest::ContentTypeHeader, "application/json");

    if (!apiKey.isEmpty ())
    {
        request.setRawHeader ("Authorization",
                              "Bearer " + apiKey.toUtf8 ());
    }

    QNetworkReply *reply = m_nam.get (request);

    connect (reply, &QNetworkReply::finished, this,
             [reply, callback] ()
             {
                 reply->deleteLater ();

                 if (reply->error () != QNetworkReply::NoError)
                 {
                     callback (false, reply->errorString (), {});
                     return;
                 }

                 const QByteArray body = reply->readAll ();
                 const QJsonDocument doc = QJsonDocument::fromJson (body);

                 if (!doc.isObject () || !doc.object ().contains ("data"))
                 {
                     callback (false,
                               "Response is not an OpenAI-compatible "
                               "/models payload",
                               {});
                     return;
                 }

                 QStringList models;

                 for (const auto &item : doc.object ()["data"].toArray ())
                 {
                     const QString id = item.toObject ()["id"].toString ();

                     if (!id.isEmpty ())
                     {
                         models.append (id);
                     }
                 }

                 callback (true, QString (), models);
             });
}
