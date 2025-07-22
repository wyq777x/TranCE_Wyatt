#include "Setting.h"
#include "View/SettingPage.h"

void Setting::initConnections ()
{
    connect (&AppSettingModel::getInstance (),
             &AppSettingModel::languageChanged, this,
             &Setting::onLanguageChanged);

    connect (&AppSettingModel::getInstance (),
             &AppSettingModel::historySearchListEnabledChanged, this,
             &Setting::onHistorySearchListEnabledChanged);
}
ChangeResult Setting::setHistorySearchEnabled (bool enabled)
{
    auto result =
        AppSettingModel::getInstance ().setHistorySearchListEnabled (enabled);

    return result;
}

ChangeResult Setting::setLanguage (const QString &lang)
{
    auto result = AppSettingModel::getInstance ().setLanguage (lang);

    return result;
}

void Setting::onLanguageChanged (const QString &lang)
{
    // Building...

    // change UI display

    qDebug () << "Language Pack" << lang << "loaded";
}

void Setting::onHistorySearchListEnabledChanged (bool enabled)
{
    // Building...

    // change UI diaplay

    qDebug () << "History search list enabled set to" << enabled;
}