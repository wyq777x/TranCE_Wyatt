#include "SettingPage.h"

#include <QPropertyAnimation>
#include <qboxlayout.h>
#include <qnamespace.h>

SettingPage::SettingPage (QWidget *parent) : TempPage (parent)
{
    setWindowTitle ("Setting Page");

    auto *centralWidget = new QWidget (this);
    centralWidget->setWindowTitle ("Setting Page");

    QVBoxLayout *settingPageLayout = new QVBoxLayout (centralWidget);
    settingPageLayout->setSpacing (20);
    settingPageLayout->setContentsMargins (0, 50, 0, 10);
    settingPageLayout->setAlignment (Qt::AlignTop);

    QHBoxLayout *enableHistorySearchLayout = new QHBoxLayout (centralWidget);

    QLabel *enableHistorySearchLabel =
        new QLabel (HISTORY_SEARCH_TEXT, centralWidget);
    enableHistorySearchLabel->setStyleSheet (
        "font-size: 20px; font-weight: normal; color: #333;");
    enableHistorySearchLabel->setSizePolicy (QSizePolicy::Expanding,
                                             QSizePolicy::Preferred);
    enableHistorySearchLabel->setFont (QFont ("Noto Sans", 24));

    enableHistorySearchLayout->addWidget (enableHistorySearchLabel);

    m_statusLabel = new QLabel (STATUS_ON, centralWidget);
    m_statusLabel->setStyleSheet (
        "font-size: 16px; font-weight: bold; color: #4CAF50;");
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

    QLabel *languageLabel = new QLabel (LANGUAGE_SETTING_TEXT, centralWidget);
    languageLabel->setStyleSheet (
        "font-size: 20px; font-weight: normal; color: #333;");
    languageLabel->setSizePolicy (QSizePolicy::Expanding,
                                  QSizePolicy::Preferred);
    languageLabel->setFont (QFont ("Noto Sans", 24));

    languageLayout->addWidget (languageLabel);

    m_languageComboBox = new ElaComboBox (centralWidget);

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
        new QLabel (QString ("Clear Cache: %1 MB").arg (0), centralWidget);
    clearCacheLabel->setStyleSheet (
        "font-size: 20px; font-weight: normal; color: #333;");
    clearCacheLabel->setFont (QFont ("Noto Sans", 24));

    clearCacheLayout->addWidget (clearCacheLabel);

    ElaPushButton *clearCacheButton =
        new ElaPushButton ("Clear Cache", centralWidget);
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

    m_statusLabel->setText (enabled ? STATUS_ON : STATUS_OFF);
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

void SettingPage::saveHistorySearchSetting (bool enabled) {}

void SettingPage::onLanguageChanged (int index)
{
    // 0: Chinese, 1: English
    QString selectedLanguage = (index == 0) ? "Chinese" : "English";
}
