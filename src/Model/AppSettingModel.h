#pragma once
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

    bool isHistoryListEnabled () const { return true; } // Placeholder

private:
    AppSettingModel () = default;

    QString language = "en_US";
};