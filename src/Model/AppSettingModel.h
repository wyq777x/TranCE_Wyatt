#pragma once

#include "Utility/Result.h"
#include <QCoreApplication>
#include <QString>

class AppSettingModel
{
public:
    static AppSettingModel &getInstance ()
    {
        static AppSettingModel instance;
        return instance;
    }
    AppSettingModel (const AppSettingModel &) = delete;
    AppSettingModel &operator= (const AppSettingModel &) = delete;
    AppSettingModel (AppSettingModel &&) = delete;
    AppSettingModel &operator= (AppSettingModel &&) = delete;

    ChangeResult setLanguage (const QString &lang)
    {
        // Building...
        if (lang.isEmpty ())
        {
            logErr ("Language cannot be empty",
                    std::runtime_error ("Invalid language"));

            return ChangeResult::NullValue;
        }

        if (lang == language)
        {
            logErr ("Language is already set to " + lang.toStdString (),
                    std::runtime_error ("No change needed"));
            return ChangeResult::StillSame;
        }
        if (lang != "en_US" && lang != "en_GB" && lang != "zh_CN" &&
            lang != "zh_TW" && lang != "zh_HK")
        {
            logErr ("Unsupported/Invalid language code: " + lang.toStdString (),
                    std::runtime_error ("Invalid language code"));
            return ChangeResult::InvalidInput;
        }
        language = lang;

        qDebug () << "Language set to" << language;
        return ChangeResult::Success;
    }

    ChangeResult setHistorySearchListEnabled (bool enabled)
    {
        // Building...
        if (enabled == historySearchEnabled)
        {
            logErr ("History search list is already set to " +
                        QString::number (enabled).toStdString (),
                    std::runtime_error ("No change needed"));
            return ChangeResult::StillSame;
        }

        historySearchEnabled = enabled;

        qDebug () << "History search list enabled set to" << enabled;
        return ChangeResult::Success;
    }

    QString getLanguage () const { return language; }

    bool isHistoryListEnabled () const { return historySearchEnabled; }

private:
    AppSettingModel () = default;

    bool historySearchEnabled = true;

    QString language = "en_US";
    // en_US : English
    // zh_CN : Chinese

    mutable std::string m_lastError;

    template <typename ExceptionT>
    void logErr (const std::string &errMsg, const ExceptionT &e) const
    {
        qCritical () << "[AppSettingModel]" << QString::fromStdString (errMsg)
                     << "Exception:" << e.what ();
        m_lastError = errMsg + " Exception:" + std::string (e.what ());
    }
};