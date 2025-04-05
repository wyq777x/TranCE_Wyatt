#include "HistoryPage.h"
#include <qboxlayout.h>

HistoryPage::HistoryPage (QWidget *parent) : TempPage (parent)
{
    setWindowTitle ("History");

    auto *centralWidget = new QWidget (this);
    centralWidget->setWindowTitle ("History");
    QVBoxLayout *layout = new QVBoxLayout (centralWidget);
    layout->setContentsMargins (3, 5, 3, 5);
    layout->setSpacing (5);

    addCentralWidget (centralWidget, true, true, 0);
}