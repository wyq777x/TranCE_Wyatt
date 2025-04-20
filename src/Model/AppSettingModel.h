#pragma once

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

private:
    AppSettingModel () = default;
};