#include "SettingPage.h"

SettingPage::SettingPage (QWidget *parent) : TempPage (parent)
{
    setWindowTitle ("Setting Page");

    auto *centralWidget = new QWidget (this);
    centralWidget->setWindowTitle ("Setting Page");

    addCentralWidget (centralWidget, true, true, 0);
}