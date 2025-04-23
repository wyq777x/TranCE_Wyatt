#include "HistoryPage.h"
#include "ElaListView.h"
#include <qboxlayout.h>
#include <qnamespace.h>

HistoryPage::HistoryPage (QWidget *parent) : TempPage (parent)
{
    setWindowTitle ("History");

    auto *centralWidget = new QWidget (this);
    centralWidget->setWindowTitle ("History");
    QVBoxLayout *layout = new QVBoxLayout (centralWidget);
    layout->setContentsMargins (3, 5, 3, 5);
    layout->setSpacing (5);
    layout->setAlignment (Qt::AlignCenter | Qt::AlignTop);

    ElaListView *searchHistoryListView = new ElaListView (centralWidget);
    searchHistoryListView->setMinimumHeight (200);
    searchHistoryListView->setMaximumHeight (300);
    searchHistoryListView->setIsTransparent (true);

    layout->addWidget (searchHistoryListView);

    ElaListView *reciteHistoryListView = new ElaListView (centralWidget);
    reciteHistoryListView->setMinimumHeight (200);
    reciteHistoryListView->setMaximumHeight (300);
    reciteHistoryListView->setIsTransparent (true);

    layout->addWidget (reciteHistoryListView);

    addCentralWidget (centralWidget, true, true, 0);
}