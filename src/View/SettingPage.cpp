#include "SettingPage.h"
#include "Utility/Constants.h"
#include "Utility/Result.h"

#include <QPropertyAnimation>
#include <qboxlayout.h>
#include <qnamespace.h>

SettingPage::SettingPage (QWidget *parent) : TempPage (parent)
{
    setWindowTitle (tr ("Setting Page"));

    auto *centralWidget = new QWidget (this);
    centralWidget->setWindowTitle (tr ("Setting Page"));

    QVBoxLayout *settingPageLayout = new QVBoxLayout (centralWidget);
    settingPageLayout->setSpacing (20);
    settingPageLayout->setContentsMargins (0, 50, 0, 10);
    settingPageLayout->setAlignment (Qt::AlignTop);

    QHBoxLayout *enableHistorySearchLayout = new QHBoxLayout (centralWidget);

    QLabel *enableHistorySearchLabel =
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

    m_historySearchSwitch = new ElaToggleSwitch (centralWidget);
    m_historySearchSwitch->setFixedSize (60, 30);
    m_historySearchSwitch->setIsToggled (true);
    enableHistorySearchLayout->addWidget (m_historySearchSwitch);

    settingPageLayout->addLayout (enableHistorySearchLayout);

    auto *splitLine1 = new QFrame (centralWidget);
    splitLine1->setFrameShape (QFrame::HLine);
    splitLine1->setFrameShadow (QFrame::Sunken);
    settingPageLayout->addWidget (splitLine1);

    // Language setting layout
    QHBoxLayout *languageLayout = new QHBoxLayout (centralWidget);

    QLabel *languageLabel =
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

    auto *splitLine2 = new QFrame (centralWidget);
    splitLine2->setFrameShape (QFrame::HLine);
    splitLine2->setFrameShadow (QFrame::Sunken);
    settingPageLayout->addWidget (splitLine2);

    QHBoxLayout *clearCacheLayout = new QHBoxLayout (centralWidget);

    QLabel *clearCacheLabel =
        new QLabel (tr ("Clear Cache: %1 MB").arg (0), centralWidget);
    clearCacheLabel->setStyleSheet (
        QString ("font-size: %1px; font-weight: normal; color: #333;")
            .arg (Constants::Settings::SUBTITLE_FONT_SIZE));
    clearCacheLabel->setFont (QFont (Constants::Settings::DEFAULT_FONT_FAMILY,
                                     Constants::Settings::SUBTITLE_FONT_SIZE,
                                     QFont::Normal));

    clearCacheLayout->addWidget (clearCacheLabel);

    ElaPushButton *clearCacheButton =
        new ElaPushButton (tr ("Clear Cache"), centralWidget);
    clearCacheButton->setMinimumWidth (150);
    clearCacheButton->setMaximumWidth (150);

    clearCacheLayout->addWidget (clearCacheButton);

    settingPageLayout->addLayout (clearCacheLayout);

    addCentralWidget (centralWidget, true, true, 0);
    connect (m_historySearchSwitch, &ElaToggleSwitch::toggled, this,
             &SettingPage::onHistorySearchToggled);
    connect (m_languageComboBox,
             QOverload<int>::of (&ElaComboBox::currentIndexChanged), this,
             &SettingPage::onLanguageChanged);
}

void SettingPage::onHistorySearchToggled (bool enabled)
{

    m_statusLabel->setText (enabled ? Constants::UI::STATUS_ON
                                    : Constants::UI::STATUS_OFF);
    m_statusLabel->setStyleSheet (
        QString ("font-size: 16px; font-weight: bold; color: %1;")
            .arg (enabled ? "#4CAF50" : "#F44336"));

    updateStatusWithAnimation (enabled);

    saveHistorySearchSetting (enabled);
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

void SettingPage::saveHistorySearchSetting (bool enabled)
{
    // Building...
    if (enabled)
    {

        // Adjust settings to enable history search
        auto result = Setting::getInstance ().setHistorySearchEnabled (enabled);
        if (result == ChangeResult::Success)
        {
            qDebug () << "History search enabled successfully.";
        }

        else
        {
            auto it = ChangeResultMessage.find (result);
            QString errorMsg = it != ChangeResultMessage.end ()
                                   ? QString::fromStdString (it->second)
                                   : "Unknown error";
            showDialog ("Save Error", errorMsg);
        }
        // save to user settings
    }
}

void SettingPage::onLanguageChanged (int index)
{

    // Building...

    // 0: Chinese, 1: English
    QString selectedLanguage = (index == 0) ? "Chinese" : "English";

    // Set the application language

    // Save the selected language to user settings
}
