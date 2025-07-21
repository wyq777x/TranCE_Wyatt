#pragma once
#include "TempPage.h"
#include "Utility/Constants.h"
#include <Controller/Setting.h>
#include <QLabel>
#include <QPropertyAnimation>


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
    void updateStatusWithAnimation (bool enabled);
    void saveHistorySearchSetting (bool enabled);

    QLabel *m_statusLabel;
    ElaToggleSwitch *m_historySearchSwitch;
    ElaComboBox *m_languageComboBox;
};