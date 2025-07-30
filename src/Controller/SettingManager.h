#pragma once

#include "Model/AppSettingModel.h"
#include "Utility/Result.h"

class SettingManager : public QObject
{
    Q_OBJECT
public:
    static SettingManager &getInstance ()
    {
        static SettingManager instance;
        return instance;
    }

    void initConnections ();

    bool isHistorySearchEnabled () const
    {
        return AppSettingModel::getInstance ().isHistoryListEnabled ();
    }

    ChangeResult setHistorySearchListEnabled (bool enabled);

    QString getLanguage () const
    {
        return AppSettingModel::getInstance ().getLanguage ();
    }

    ChangeResult setLanguage (const QString &lang);

private slots:
    void onLanguageChanged (const QString &lang);

private:
    explicit SettingManager () { initConnections (); }
    SettingManager (const SettingManager &) = delete;
    SettingManager &operator= (const SettingManager &) = delete;
    SettingManager (SettingManager &&) = delete;
    SettingManager &operator= (SettingManager &&) = delete;

    mutable std::string m_lastError;

    template <typename ExceptionT>
    void logErr (const std::string &errMsg, const ExceptionT &e) const
    {
        qCritical () << "[SettingManager]" << QString::fromStdString (errMsg)
                     << "Exception:" << e.what ();
    }
};