#include "AboutPage.h"

AboutPage::AboutPage(QWidget *parent)
    : TempPage(parent)
{
    setWindowTitle("About");

    auto *centralWidget = new QWidget(this);
    centralWidget->setWindowTitle("About");

    addCentralWidget(centralWidget, true, true, 0);
}