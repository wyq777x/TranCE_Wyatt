#include "HistoryPage.h"

HistoryPage::HistoryPage(QWidget *parent)
    : TempPage(parent)
{
    setWindowTitle("History");

    auto *centralWidget = new QWidget(this);
    centralWidget->setWindowTitle("History");

    addCentralWidget(centralWidget, true, true, 0);
}