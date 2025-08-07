#include "HistoryPage.h"
#include "Controller/AccountManager.h"
#include "Controller/DbManager.h"
#include "Controller/UIController.h"

HistoryPage::HistoryPage (QWidget *parent) : TempPage (parent)
{
    setWindowTitle (tr ("History"));

    initUI ();
    initConnections ();
}

void HistoryPage::enableHistorySearchListUI (bool enabled)
{
    if (searchHistoryListView)
    {
        searchHistoryListView->setEnabled (enabled);
        searchHistoryListView->setVisible (enabled);
        searchHistoryLabel->setVisible (enabled);
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

    searchHistoryLabel = new QLabel (tr ("Search History"), centralWidget);
    searchHistoryLabel->setStyleSheet (
        "font-size: 20px; font-weight: normal; color: #333;");
    searchHistoryLabel->setFont (
        QFont (Constants::Settings::DEFAULT_FONT_FAMILY,
               Constants::Settings::TITLE_FONT_SIZE));

    layout->addWidget (searchHistoryLabel);

    searchHistoryListView = new ElaListView (centralWidget);
    searchHistoryListView->setFixedSize (550, 300);
    searchHistoryListView->setIsTransparent (true);

    historySearchModel = new QStringListModel (this);
    searchHistoryListView->setModel (historySearchModel);

    layout->addWidget (searchHistoryListView);

    reciteHistoryLabel = new QLabel (tr ("Recite History"), centralWidget);
    reciteHistoryLabel->setStyleSheet (
        "font-size: 20px; font-weight: normal; color: #333;");
    reciteHistoryLabel->setFont (
        QFont (Constants::Settings::DEFAULT_FONT_FAMILY,
               Constants::Settings::TITLE_FONT_SIZE));

    layout->addWidget (reciteHistoryLabel);

    reciteHistoryListView = new ElaListView (centralWidget);
    reciteHistoryListView->setFixedSize (550, 300);
    reciteHistoryListView->setIsTransparent (true);
    layout->addWidget (reciteHistoryListView);

    addCentralWidget (centralWidget, true, true, 0);

    loadSearchHistory ();
}

void HistoryPage::initConnections ()
{
    connect (&UIController::getInstance (),
             &UIController::historySearchListUIChanged, this,
             &HistoryPage::enableHistorySearchListUI);

    connect (&AccountManager::getInstance (), &AccountManager::loginSuccessful,
             this, [this] (const QString &) { loadSearchHistory (); });

    connect (&AccountManager::getInstance (), &AccountManager::logoutSuccessful,
             this,
             [this] ()
             {
                 // Clear history when user logs out
                 if (historySearchModel)
                 {
                     historySearchModel->setStringList (QStringList ());
                 }
             });

    connect (&UIController::getInstance (), &UIController::searchHistoryUpdated,
             this, [this] () { loadSearchHistory (); });

    connect (
        searchHistoryListView, &ElaListView::clicked, this,
        [this] (const QModelIndex &index)
        {
            auto word = index.data ().toString ();
            auto wordEntry = DbManager::getInstance ().lookupWord (word, "en");
            if (wordEntry.has_value ())
            {
                UIController::getInstance ().showWordCard (wordEntry.value ());
            }
        });
}

void HistoryPage::loadSearchHistory ()
{
    if (AccountManager::getInstance ().isLoggedIn ())
    {
        auto userId = AccountManager::getInstance ().getUserUuid (
            AccountManager::getInstance ().getUsername ());

        auto historyVector =
            DbManager::getInstance ().getUserHistorySearchVector (userId);

        QStringList historyStringList;
        for (const auto &wordEntry : historyVector)
        {
            historyStringList << wordEntry.word;
        }

        historySearchModel->setStringList (historyStringList);
    }
}