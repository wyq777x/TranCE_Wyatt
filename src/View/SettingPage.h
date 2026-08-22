#pragma once

#include "Utility/AiProviderConfig.h"
#include "Utility/Constants.h"
#include "Utility/Result.h"
#include "View/TempPage.h"

class ElaComboBox;
class ElaPushButton;
class ElaToggleSwitch;
class QFrame;
class QHBoxLayout;
class QLabel;
class QListWidget;
class QListWidgetItem;
class QShowEvent;
class QVBoxLayout;
class QWidget;

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

protected:
    void showEvent (QShowEvent *event) override;

private slots:
    void onHistorySearchListEnabledToggled (bool enabled);
    void onLanguageChanged (int index);
    void onClearCacheClicked ();
    void onAiModeToggled (bool enabled);
    void onAddProviderClicked ();
    void onEditProviderClicked ();
    void onRemoveProviderClicked ();
    void onProviderItemActivated (QListWidgetItem *item);

private:
    void initUI ();
    void initConnections ();
    void updateStatusWithAnimation (bool enabled);
    void refreshCacheLabel ();
    void refreshAiProviderList ();
    void setAiProviderControlsEnabled (bool enabled);
    static QString formatBytes (std::size_t bytes);

    // change AppSettingModel through SettingManager controller
    ChangeResult changeHistorySearchListEnabled (bool enabled);
    ChangeResult changeAiModeEnabled (bool enabled);

    // Change UserJson data of UserModel through the AccountManager controller
    ChangeResult
    changeHistorySearchListEnabled_Json (bool enabled,
                                         const QString &userProfile);

    ChangeResult changeAiModeEnabled_Json (bool enabled,
                                           const QString &userProfile);

    ChangeResult changeLanguage_Json (const QString &lang,
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
    QFrame *splitLine3;
    QHBoxLayout *languageLayout;
    QLabel *languageLabel;
    ElaComboBox *m_languageComboBox;
    QHBoxLayout *clearCacheLayout;
    QLabel *clearCacheLabel;
    ElaPushButton *clearCacheButton;

    // AI mode section
    QHBoxLayout *aiModeLayout = nullptr;
    QLabel *aiModeLabel = nullptr;
    QLabel *m_aiModeStatusLabel = nullptr;
    ElaToggleSwitch *m_aiModeSwitch = nullptr;
    QListWidget *m_aiProviderList = nullptr;
    ElaPushButton *m_addProviderButton = nullptr;
    ElaPushButton *m_editProviderButton = nullptr;
    ElaPushButton *m_removeProviderButton = nullptr;
};
