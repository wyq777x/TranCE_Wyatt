#include "AppSettingModel.h"

ChangeResult AppSettingModel::setLanguage (const QString &lang)
{
    // Building...
    if (lang.isEmpty ())
    {
        logErr ("Language cannot be empty",
                std::runtime_error ("Invalid language"));

        return ChangeResult::NullValue;
    }

    if (lang != "en_US" && lang != "en_GB" && lang != "zh_CN" &&
        lang != "zh_TW" && lang != "zh_HK")
    {
        logErr ("Unsupported/Invalid language code: " + lang.toStdString (),
                std::runtime_error ("Invalid language code"));
        return ChangeResult::InvalidInput;
    }
    language = lang;

    emit languageChanged (lang);

    qDebug () << "Language set to" << language;
    return ChangeResult::Success;
}

ChangeResult AppSettingModel::setHistorySearchListEnabled (bool enabled)
{

    historySearchEnabled = enabled;

    emit historySearchListEnabledChanged (enabled);

    qDebug () << "History search list enabled set to" << enabled;
    return ChangeResult::Success;
}