#include "StatisticsPage.h"

StatisticsPage::StatisticsPage(QWidget *parent)
    : TempPage(parent)
{
    setWindowTitle("Statistics");

    auto *centralWidget = new QWidget(this);
    centralWidget->setWindowTitle("Statistics");

    addCentralWidget(centralWidget, true, true, 0);
}