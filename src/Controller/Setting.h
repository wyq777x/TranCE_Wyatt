#pragma once

#include "SettingPage.h"

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
};