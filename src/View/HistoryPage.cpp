#include "HistoryPage.h"
#include "Controller/UIController.h"

HistoryPage::HistoryPage (QWidget *parent) : TempPage (parent)
{
    setWindowTitle (tr ("History"));

    initUI ();
    initConnections ();
}

void HistoryPage::enableHistorySearchListUI (bool enable)
{
    if (searchHistoryListView)
    {
        searchHistoryListView->setEnabled (enable);
    }
}

void HistoryPage::initUI ()
{
    centralWidget = new QWidget (this);
    centralWidget->setWindowTitle (tr ("History"));

    layout = new QVBoxLayout (centralWidget);
    layout->setContentsMargins (3, 5, 3, 5);
    layout->setSpacing (5);
    layout->setAlignment (Qt::AlignCenter | Qt::AlignTop);

    searchHistoryListView = new ElaListView (centralWidget);
    searchHistoryListView->setMinimumHeight (200);
    searchHistoryListView->setMaximumHeight (300);
    searchHistoryListView->setIsTransparent (true);
    layout->addWidget (searchHistoryListView);

    reciteHistoryListView = new ElaListView (centralWidget);
    reciteHistoryListView->setMinimumHeight (200);
    reciteHistoryListView->setMaximumHeight (300);
    reciteHistoryListView->setIsTransparent (true);
    layout->addWidget (reciteHistoryListView);

    addCentralWidget (centralWidget, true, true, 0);
}

void HistoryPage::initConnections ()
{
    // Building...
    connect (&UIController::getInstance (),
             &UIController::historySearchListUIChanged, this,
             &HistoryPage::enableHistorySearchListUI);
}