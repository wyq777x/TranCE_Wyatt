#pragma once

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
    QString getLanguage () const { return language; }

    bool isHistoryListEnabled () const { return historySearchEnabled; }

private:
    AppSettingModel () = default;

    bool historySearchEnabled = true;
    QString language = "en_US";

    mutable std::string m_lastError;

    template <typename ExceptionT>
    void logErr (const std::string &errMsg, const ExceptionT &e) const
    {
        qCritical () << "[AppSettingModel]" << QString::fromStdString (errMsg)
                     << "Exception:" << e.what ();
        m_lastError = errMsg + " Exception:" + std::string (e.what ());
    }
};