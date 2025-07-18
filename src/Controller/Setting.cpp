#include "Setting.h"

ChangeResult Setting::setHistorySearchEnabled (bool enabled)
{
    // Building...
    auto result =
        AppSettingModel::getInstance ().setHistorySearchListEnabled (enabled);

    return result;
}