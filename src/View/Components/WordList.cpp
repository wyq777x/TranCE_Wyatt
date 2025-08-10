#include "WordList.h"
#include "Controller/DbManager.h"
#include "Controller/UIController.h"

WordList::WordList (QWidget *parent) : TempPage (parent)
{
    setWindowTitle (tr ("Word List"));
    setStyleSheet (
        "background-color: #f0f0f0; font-family: 'Noto Sans', sans-serif;");

    initUI ();
    initConnections ();
}

void WordList::setTitle (const QString &title)
{
    currentTitle = title;
    setWindowTitle (title);
}

void WordList::setWordList (const QStringList &words)
{
    currentWords = words;
    if (wordListModel)
    {
        wordListModel->setStringList (words);
    }
}

void WordList::onWordClicked (const QModelIndex &index)
{
    auto word = index.data ().toString ();
    auto wordEntry = DbManager::getInstance ().lookupWord (word, "en");
    if (wordEntry.has_value ())
    {
        UIController::getInstance ().showWordCard (wordEntry.value ());
    }
}

void WordList::initUI ()
{
    centralWidget = new QWidget (this);
    centralWidget->setWindowTitle (currentTitle.isEmpty () ? tr ("Word List")
                                                           : currentTitle);

    mainLayout = new QVBoxLayout (centralWidget);
    mainLayout->setContentsMargins (20, 20, 20, 20);
    mainLayout->setSpacing (15);
    mainLayout->setAlignment (Qt::AlignCenter | Qt::AlignTop);

    wordListView = new ElaListView (centralWidget);
    wordListView->setFixedSize (600, 450);
    wordListView->setIsTransparent (true);
    wordListView->setEditTriggers (QAbstractItemView::NoEditTriggers);

    // Apply similar style to WordCard
    wordListView->setStyleSheet (
        QString ("ElaListView {"
                 "background-color: #ffffff;"
                 "border: 1px solid #ecf0f1;"
                 "border-radius: 8px;"
                 "font-size: %1px;"
                 "font-family: '%2';"
                 "color: #34495e;"
                 "}")
            .arg (Constants::Settings::DEFAULT_FONT_SIZE)
            .arg (Constants::Settings::DEFAULT_FONT_FAMILY));

    wordListModel = new QStringListModel (this);
    wordListView->setModel (wordListModel);

    if (!currentWords.isEmpty ())
    {
        wordListModel->setStringList (currentWords);
    }

    mainLayout->addWidget (wordListView);

    addCentralWidget (centralWidget, true, true, 0);
}

void WordList::initConnections ()
{
    connect (wordListView, &ElaListView::clicked, this,
             &WordList::onWordClicked);
}
