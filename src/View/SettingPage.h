#pragma once
#include "TempPage.h"
#include <QLabel>
#include <QPropertyAnimation>

class ElaToggleSwitch;

namespace
{
const QString HISTORY_SEARCH_TEXT = "Enable History Search List";
const QString STATUS_ON = "ON";
const QString STATUS_OFF = "OFF";
} // namespace

class SettingPage : public TempPage
{
    Q_OBJECT
public:
    explicit SettingPage (QWidget *parent = nullptr);

    QString createHistorySearchText (bool enabled)
    {
        return QString ("%1: %2").arg (HISTORY_SEARCH_TEXT,
                                       enabled ? STATUS_ON : STATUS_OFF);
    }

private slots:
    void onHistorySearchToggled (bool enabled);

private:
    void updateStatusWithAnimation (bool enabled);
    void saveHistorySearchSetting (bool enabled);

    QLabel *m_statusLabel;
    ElaToggleSwitch *m_historySearchSwitch;
};