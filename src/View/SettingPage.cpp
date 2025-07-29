#include "SettingPage.h"
#include "Controller/AccountManager.h"
#include "Utility/Constants.h"
#include "Utility/Result.h"

SettingPage::SettingPage (QWidget *parent) : TempPage (parent)
{
    setWindowTitle (tr ("Setting Page"));

    initUI ();
    initConnections ();
}

void SettingPage::initUI ()
{
    centralWidget = new QWidget (this);
    centralWidget->setWindowTitle (tr ("Setting Page"));

    settingPageLayout = new QVBoxLayout (centralWidget);
    settingPageLayout->setSpacing (20);
    settingPageLayout->setContentsMargins (0, 50, 0, 10);
    settingPageLayout->setAlignment (Qt::AlignTop);

    enableHistorySearchLayout = new QHBoxLayout (centralWidget);

    enableHistorySearchLabel =
        new QLabel (Constants::UI::HISTORY_SEARCH_TEXT, centralWidget);
    enableHistorySearchLabel->setStyleSheet (
        QString ("font-size: %1px; font-weight: normal; color: #333;")
            .arg (Constants::Settings::SUBTITLE_FONT_SIZE));
    enableHistorySearchLabel->setSizePolicy (QSizePolicy::Expanding,
                                             QSizePolicy::Preferred);
    enableHistorySearchLabel->setFont (
        QFont (Constants::Settings::DEFAULT_FONT_FAMILY,
               Constants::Settings::SUBTITLE_FONT_SIZE, QFont::Normal));
    enableHistorySearchLayout->addWidget (enableHistorySearchLabel);

    m_statusLabel = new QLabel (Constants::UI::STATUS_ON, centralWidget);
    m_statusLabel->setStyleSheet (
        QString ("font-size: %1px; font-weight: bold; color: #4CAF50;")
            .arg (Constants::Settings::DEFAULT_FONT_SIZE));
    m_statusLabel->setAlignment (Qt::AlignCenter);
    m_statusLabel->setMinimumWidth (40);
    enableHistorySearchLayout->addWidget (m_statusLabel);

    m_historySearchListEnabledSwitch = new ElaToggleSwitch (centralWidget);
    m_historySearchListEnabledSwitch->setFixedSize (60, 30);
    m_historySearchListEnabledSwitch->setIsToggled (true);
    enableHistorySearchLayout->addWidget (m_historySearchListEnabledSwitch);

    settingPageLayout->addLayout (enableHistorySearchLayout);

    splitLine1 = new QFrame (centralWidget);
    splitLine1->setFrameShape (QFrame::HLine);
    splitLine1->setFrameShadow (QFrame::Sunken);
    settingPageLayout->addWidget (splitLine1);

    // Language setting layout
    languageLayout = new QHBoxLayout (centralWidget);

    languageLabel =
        new QLabel (Constants::UI::LANGUAGE_SETTING_TEXT, centralWidget);
    languageLabel->setStyleSheet (
        QString ("font-size: %1px; font-weight: normal; color: #333;")
            .arg (Constants::Settings::SUBTITLE_FONT_SIZE));
    languageLabel->setSizePolicy (QSizePolicy::Expanding,
                                  QSizePolicy::Preferred);
    languageLabel->setFont (QFont (Constants::Settings::DEFAULT_FONT_FAMILY,
                                   Constants::Settings::SUBTITLE_FONT_SIZE,
                                   QFont::Normal));
    languageLayout->addWidget (languageLabel);

    m_languageComboBox = new ElaComboBox (centralWidget);
    m_languageComboBox->setStyleSheet (
        "QComboBox {"
        "    color: black;" // set default text color as black
        "    font-family: '" +
        Constants::Settings::DEFAULT_FONT_FAMILY +
        "';"
        "}"
        "QComboBox QAbstractItemView {"
        "    color: black;" // set text color of dropdown list as black
        "}");
    m_languageComboBox->addItem ("中文");
    m_languageComboBox->addItem ("English");
    m_languageComboBox->setCurrentIndex (1); // Default to English
    languageLayout->addWidget (m_languageComboBox);

    settingPageLayout->addLayout (languageLayout);

    splitLine2 = new QFrame (centralWidget);
    splitLine2->setFrameShape (QFrame::HLine);
    splitLine2->setFrameShadow (QFrame::Sunken);
    settingPageLayout->addWidget (splitLine2);

    clearCacheLayout = new QHBoxLayout (centralWidget);

    clearCacheLabel =
        new QLabel (tr ("Clear Cache: %1 MB").arg (0), centralWidget);
    clearCacheLabel->setStyleSheet (
        QString ("font-size: %1px; font-weight: normal; color: #333;")
            .arg (Constants::Settings::SUBTITLE_FONT_SIZE));
    clearCacheLabel->setFont (QFont (Constants::Settings::DEFAULT_FONT_FAMILY,
                                     Constants::Settings::SUBTITLE_FONT_SIZE,
                                     QFont::Normal));
    clearCacheLayout->addWidget (clearCacheLabel);

    clearCacheButton = new ElaPushButton (tr ("Clear Cache"), centralWidget);
    clearCacheButton->setMinimumWidth (150);
    clearCacheButton->setMaximumWidth (150);
    clearCacheLayout->addWidget (clearCacheButton);

    settingPageLayout->addLayout (clearCacheLayout);

    addCentralWidget (centralWidget, true, true, 0);
}

void SettingPage::initConnections ()
{
    connect (m_historySearchListEnabledSwitch, &ElaToggleSwitch::toggled, this,
             &SettingPage::onHistorySearchListEnabledToggled);
    connect (m_languageComboBox,
             QOverload<int>::of (&ElaComboBox::currentIndexChanged), this,
             &SettingPage::onLanguageChanged);
}

void SettingPage::onHistorySearchListEnabledToggled (bool enabled)
{

    m_statusLabel->setText (enabled ? Constants::UI::STATUS_ON
                                    : Constants::UI::STATUS_OFF);
    m_statusLabel->setStyleSheet (
        QString ("font-size: 16px; font-weight: bold; color: %1;")
            .arg (enabled ? "#4CAF50" : "#F44336"));

    updateStatusWithAnimation (enabled);

    if (AccountManager::getInstance ().isLoggedIn ())
    {
        auto changeJsonResult = changeHistorySearchListEnabled_Json (
            enabled, "profile_" +
                         AccountManager::getInstance ().getUserUuid (
                             AccountManager::getInstance ().getUsername ()) +
                         ".json");

        if (changeJsonResult != ChangeResult::Success)
        {
            QString errorMsg = QString::fromStdString (
                getErrorMessage (changeJsonResult, ChangeResultMessage));
            showDialog (tr ("Error"), errorMsg);
        }
        else
        {
            qDebug ()
                << "History search list enabled JSON changed successfully to"
                << enabled;
        }

        // Block signals to prevent recursive invocations
        m_historySearchListEnabledSwitch->blockSignals (true);

        auto changeSettingsResult = changeHistorySearchListEnabled (enabled);

        if (changeSettingsResult != ChangeResult::Success)
        {
            QString errorMsg = QString::fromStdString (
                getErrorMessage (changeSettingsResult, ChangeResultMessage));
            showDialog (tr ("Error"), errorMsg);

            // fallback to previous state
            m_historySearchListEnabledSwitch->setIsToggled (!enabled);
            m_statusLabel->setText (enabled ? Constants::UI::STATUS_OFF
                                            : Constants::UI::STATUS_ON);
        }

        m_historySearchListEnabledSwitch->blockSignals (false);
    }
    else
    {
        // If not logged in, just change the AppSettingModel
        auto changeSettingsResult = changeHistorySearchListEnabled (enabled);

        if (changeSettingsResult != ChangeResult::Success)
        {
            QString errorMsg = QString::fromStdString (
                getErrorMessage (changeSettingsResult, ChangeResultMessage));
            showDialog (tr ("Error"), errorMsg);

            // fallback to previous state
            m_historySearchListEnabledSwitch->setIsToggled (!enabled);
            m_statusLabel->setText (enabled ? Constants::UI::STATUS_OFF
                                            : Constants::UI::STATUS_ON);
        }
    }
}

void SettingPage::updateStatusWithAnimation (bool enabled)
{

    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect (this);
    m_statusLabel->setGraphicsEffect (effect);

    QPropertyAnimation *animation = new QPropertyAnimation (effect, "opacity");
    animation->setDuration (200);
    animation->setStartValue (0.3);
    animation->setEndValue (1.0);
    animation->start (QAbstractAnimation::DeleteWhenStopped);
}

ChangeResult SettingPage::changeHistorySearchListEnabled (bool enabled)
{
    auto result =
        SettingManager::getInstance ().setHistorySearchListEnabled (enabled);

    return result;
}

ChangeResult
SettingPage::changeHistorySearchListEnabled_Json (bool enabled,
                                                  const QString &userProfile)
{
    auto result =
        AccountManager::getInstance ().changeHistorySearchListEnabled_Json (
            enabled, userProfile);

    return result;
}

ChangeResult SettingPage::changeLanguage_Json (const QString &lang,
                                               const QString &userProfile)
{
    // Building...

    auto result =
        AccountManager::getInstance ().changeLanguage_Json (lang, userProfile);

    return result;
}
void SettingPage::onLanguageChanged (int index)
{

    // Building...

    // 0: Chinese, 1: English
    QString selectedLanguage = (index == 0) ? "zh_CN" : "en_US";

    // Set the application language

    // Save the selected language to user settings
    if (AccountManager::getInstance ().isLoggedIn ())
    {
        auto changeJsonResult = changeLanguage_Json (
            selectedLanguage,
            "profile_" +
                AccountManager::getInstance ().getUserUuid (
                    AccountManager::getInstance ().getUsername ()) +
                ".json");

        if (changeJsonResult != ChangeResult::Success)
        {
            QString errorMsg = QString::fromStdString (
                getErrorMessage (changeJsonResult, ChangeResultMessage));
            showDialog (tr ("Error"), errorMsg);
        }
        else
        {
            qDebug () << "Language changed successfully to" << selectedLanguage;
        }
    }
}
