#include "Setting.h"
#include "View/SettingPage.h"

void Setting::initConnections ()
{
    connect (&AppSettingModel::getInstance (),
             &AppSettingModel::languageChanged, this,
             &Setting::onLanguageChanged);
}
ChangeResult Setting::setHistorySearchListEnabled (bool enabled)
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
