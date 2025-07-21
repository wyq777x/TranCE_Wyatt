#include "Setting.h"

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