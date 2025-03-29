#include "RecitePage.h"

RecitePage::RecitePage(QWidget *parent)
    : TempPage(parent)
{
    setWindowTitle("Recite Page");

    auto *centralWidget = new QWidget(this);
    centralWidget->setWindowTitle("Recite Page");

    addCentralWidget(centralWidget, true, true, 0);
}