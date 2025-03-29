#include "HomePage.h"

HomePage::HomePage(QWidget *parent)
    : TempPage(parent)
{
    setWindowTitle("Home");

    auto *centralWidget = new QWidget(this);
    centralWidget->setWindowTitle("Home");

    addCentralWidget(centralWidget, true, true, 0);
}