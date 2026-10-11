#include "AppSettingModel.h"

ChangeResult AppSettingModel::setLanguage (const QString &lang)
{
    // Building...
    if (lang.isEmpty ())
    {
        qCWarning (lcSettings) << "Language cannot be empty";

        return ChangeResult::NullValue;
    }

    if (lang != "en_US" && lang != "en_GB" && lang != "zh_CN" &&
        lang != "zh_TW" && lang != "zh_HK")
    {
        qCWarning (lcSettings) << "Unsupported/Invalid language code:" << lang;
        return ChangeResult::InvalidInput;
    }
    language = lang;

    emit languageChanged (lang);

    qCDebug (lcSettings) << "Language set to" << language;
    return ChangeResult::Success;
}

ChangeResult AppSettingModel::setHistorySearchListEnabled (bool enabled)
{

    historySearchEnabled = enabled;

    emit historySearchListEnabledChanged (enabled);

    qCDebug (lcSettings) << "History search list enabled set to" << enabled;
    return ChangeResult::Success;
}

ChangeResult AppSettingModel::setAiModeEnabled (bool enabled)
{
    aiModeEnabled = enabled;

    emit aiModeEnabledChanged (enabled);

    qCDebug (lcSettings) << "AI mode enabled set to" << enabled;
    return ChangeResult::Success;
}