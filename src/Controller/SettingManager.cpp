#include "SettingManager.h"
#include "Utility/Logging.h"

void SettingManager::initConnections ()
{
    connect (&AppSettingModel::getInstance (),
             &AppSettingModel::languageChanged, this,
             &SettingManager::onLanguageChanged);
}
ChangeResult SettingManager::setHistorySearchListEnabled (bool enabled)
{
    auto result =
        AppSettingModel::getInstance ().setHistorySearchListEnabled (enabled);

    return result;
}

ChangeResult SettingManager::setAiModeEnabled (bool enabled)
{
    auto result = AppSettingModel::getInstance ().setAiModeEnabled (enabled);

    return result;
}

ChangeResult SettingManager::setLanguage (const QString &lang)
{
    auto result = AppSettingModel::getInstance ().setLanguage (lang);

    return result;
}

void SettingManager::onLanguageChanged (const QString &lang)
{
    // Building...

    qCDebug (lcSettings) << "Language Pack" << lang << "loaded";
}
