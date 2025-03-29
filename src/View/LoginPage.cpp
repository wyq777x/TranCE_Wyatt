#include "LoginPage.h"

LoginPage::LoginPage(QWidget *parent)
    : TempPage(parent)
{
    setWindowTitle("Login");

    auto *centralWidget = new QWidget(this);
    centralWidget->setWindowTitle("Login");

    addCentralWidget(centralWidget, true, true, 0);
}