#pragma once

#include "Model/DbModel.h"
#include "SettingPage.h"
#include "Utility/Result.h"

class Setting
{
public:
    static Setting &getInstance ()
    {
        static Setting instance;
        return instance;
    }

    ChangeResult setHistorySearchEnabled (bool enabled)
    {
        // Building...
        return ChangeResult::Success;
    }

private:
    Setting () = default;
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