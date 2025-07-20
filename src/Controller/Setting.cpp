#include "Setting.h"

ChangeResult Setting::setHistorySearchEnabled (bool enabled)
{
    auto result =
        AppSettingModel::getInstance ().setHistorySearchListEnabled (enabled);

    return result;
}