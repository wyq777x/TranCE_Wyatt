#pragma once

#include "Model/AppSettingModel.h"
#include "Utility/Result.h"

class Setting : public QObject
{
    Q_OBJECT
public:
    static Setting &getInstance ()
    {
        static Setting instance;
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
    void onHistorySearchListEnabledChanged (bool enabled);

private:
    Setting () { initConnections (); }
    Setting (const Setting &) = delete;
    Setting &operator= (const Setting &) = delete;
    Setting (Setting &&) = delete;
    Setting &operator= (Setting &&) = delete;

    mutable std::string m_lastError;

    template <typename ExceptionT>
    void logErr (const std::string &errMsg, const ExceptionT &e) const
    {
        qCritical () << "[Setting]" << QString::fromStdString (errMsg)
                     << "Exception:" << e.what ();
    }
};