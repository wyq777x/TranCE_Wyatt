#pragma once

#include "Utility/Constants.h"
#include "View/TempPage.h"
#include <Controller/Setting.h>

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
    void onHistorySearchToggled (bool enabled);
    void onLanguageChanged (int index);

private:
    void initUI ();
    void initConnections ();
    void updateStatusWithAnimation (bool enabled);
    void saveHistorySearchSetting (bool enabled);

    // UI components
    QWidget *centralWidget;
    QVBoxLayout *settingPageLayout;
    QHBoxLayout *enableHistorySearchLayout;
    QLabel *enableHistorySearchLabel;
    QLabel *m_statusLabel;
    ElaToggleSwitch *m_historySearchSwitch;
    QFrame *splitLine1;
    QFrame *splitLine2;
    QHBoxLayout *languageLayout;
    QLabel *languageLabel;
    ElaComboBox *m_languageComboBox;
    QHBoxLayout *clearCacheLayout;
    QLabel *clearCacheLabel;
    ElaPushButton *clearCacheButton;
};