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
    settingPageLayout->setSpacing (20);
    settingPageLayout->setContentsMargins (0, 50, 0, 10);
    settingPageLayout->setAlignment (Qt::AlignTop);

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

    auto *splitLine1 = new QFrame (centralWidget);
    splitLine1->setFrameShape (QFrame::HLine);
    splitLine1->setFrameShadow (QFrame::Sunken);
    settingPageLayout->addWidget (splitLine1);

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
}
