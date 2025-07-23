#pragma once

#include "Utility/Result.h"
#include <QCoreApplication>
#include <QObject>
#include <QString>

class AppSettingModel : public QObject
{
    Q_OBJECT
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

    ChangeResult setLanguage (const QString &lang);

    ChangeResult setHistorySearchListEnabled (bool enabled);

    QString getLanguage () const { return language; }

    bool isHistoryListEnabled () const { return historySearchEnabled; }

signals:
    // To be used
    void languageChanged (const QString &language);
    void historySearchListEnabledChanged (bool enabled);

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