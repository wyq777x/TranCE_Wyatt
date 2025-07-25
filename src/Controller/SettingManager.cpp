#include "SettingManager.h"
#include "View/SettingPage.h"

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

ChangeResult SettingManager::setLanguage (const QString &lang)
{
    auto result = AppSettingModel::getInstance ().setLanguage (lang);

    return result;
}

void SettingManager::onLanguageChanged (const QString &lang)
{
    // Building...

    qDebug () << "Language Pack" << lang << "loaded";
}
