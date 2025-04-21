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

    QHBoxLayout *clearCacheLayout = new QHBoxLayout (centralWidget);

    QLabel *clearCacheLabel = new QLabel ("Clear Cache: ", centralWidget);
    clearCacheLabel->setAlignment (Qt::AlignLeft);
    clearCacheLabel->setStyleSheet (
        "font-size: 20px; font-weight: normal; color: #333;");
    clearCacheLabel->setFont (QFont ("Noto Sans", 24));

    clearCacheLayout->addWidget (clearCacheLabel);

    ElaPushButton *clearCacheButton =
        new ElaPushButton ("Clear Cache", centralWidget);
    clearCacheButton->setMaximumWidth (150);

    clearCacheLayout->addWidget (clearCacheButton);

    settingPageLayout->addLayout (clearCacheLayout);

    addCentralWidget (centralWidget, true, true, 0);
}