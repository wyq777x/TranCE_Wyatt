#pragma once

#include "Utility/Constants.h"
#include "View/TempPage.h"
#include <Controller/SettingManager.h>

class SettingPage : public TempPage
{
    Q_OBJECT
public:
    explicit SettingPage (QWidget *parent = nullptr);

    QString createHistorySearchText (bool enabled)
    {
        return QString ("%1: %2").arg (Constants::UI::HISTORY_SEARCH_TEXT,
                                       enabled ? Constants::UI::STATUS_ON
                                               : Constants::UI::STATUS_OFF);
    }

private slots:
    void onHistorySearchListEnabledToggled (bool enabled);
    void onLanguageChanged (int index);

private:
    void initUI ();
    void initConnections ();
    void updateStatusWithAnimation (bool enabled);

    // change AppSettingModel through SettingManager controller
    ChangeResult changeHistorySearchListEnabled (bool enabled);

    // Change UserJson data of UserModel through the AccountManager controller
    ChangeResult
    changeHistorySearchListEnabled_Json (bool enabled,
                                         const QString &userProfile);

    // UI components
    QWidget *centralWidget;
    QVBoxLayout *settingPageLayout;
    QHBoxLayout *enableHistorySearchLayout;
    QLabel *enableHistorySearchLabel;
    QLabel *m_statusLabel;
    ElaToggleSwitch *m_historySearchListEnabledSwitch;
    QFrame *splitLine1;
    QFrame *splitLine2;
    QHBoxLayout *languageLayout;
    QLabel *languageLabel;
    ElaComboBox *m_languageComboBox;
    QHBoxLayout *clearCacheLayout;
    QLabel *clearCacheLabel;
    ElaPushButton *clearCacheButton;
};