#include "SettingPage.h"
#include "ElaPushButton.h"
#include "ElaToggleSwitch.h"
#include <qboxlayout.h>
#include <qnamespace.h>

SettingPage::SettingPage (QWidget *parent) : TempPage (parent)
{
    setWindowTitle ("Setting Page");

    auto *centralWidget = new QWidget (this);
    centralWidget->setWindowTitle ("Setting Page");

    QVBoxLayout *settingPageLayout = new QVBoxLayout (centralWidget);

    QHBoxLayout *enableHistorySearchLayout = new QHBoxLayout (centralWidget);
    QLabel *enableHistorySearchLabel =
        new QLabel ("Enable History Search List: ", centralWidget);
    enableHistorySearchLabel->setStyleSheet (
        "font-size: 20px; font-weight: normal; color: #333;");
    enableHistorySearchLabel->setFont (QFont ("Noto Sans", 24));
    enableHistorySearchLayout->addWidget (enableHistorySearchLabel);

    ElaToggleSwitch *enableHistorySearchSwitch =
        new ElaToggleSwitch (centralWidget);
    enableHistorySearchSwitch->setFixedSize (60, 30);
    enableHistorySearchSwitch->setIsToggled (true);
    enableHistorySearchLayout->addWidget (enableHistorySearchSwitch);

    settingPageLayout->addLayout (enableHistorySearchLayout);

    QHBoxLayout *clearCacheLayout = new QHBoxLayout (centralWidget);

    QLabel *clearCacheLabel = new QLabel ("Clear Cache: ", centralWidget);
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
}