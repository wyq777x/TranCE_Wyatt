#include "QuizCard.h"
#include "Controller/AccountManager.h"
#include "Controller/DbManager.h"
#include "Utility/Constants.h"

QuizCard::QuizCard (QWidget *parent) : TempPage (parent)
{
    setWindowTitle (tr ("Quiz Card"));
    setFixedSize (400, 400);
    setStyleSheet (
        "background-color: #f0f0f0; font-family: 'Noto Sans', sans-serif;");

    initUI ();
    initConnections ();
}

void QuizCard::setWordEntry (WordEntry &entry)
{
    // Building...

    currentWordEntry = entry;

    wordLabel->setText (entry.word);

    updateAdd2FavoritesButton ();
}

void QuizCard::initUI ()
{
    // UI Layout:

    //   ENGLISH WORD     ADD2FAVORITES_BUTTON MASTER_BUTTON
    //   -------------------
    //   (Chinese Buttons)
    //   OPTION A
    //   OPTION B
    //   OPTION C
    //   OPTION D

    centralWidget = new QWidget (this);
    centralWidget->setWindowTitle (tr ("Quiz Card"));

    mainLayout = new QVBoxLayout (centralWidget);
    mainLayout->setContentsMargins (20, 20, 20, 20);
    mainLayout->setSpacing (10);

    headerLayout = new QHBoxLayout ();

    wordLabel = new QLabel ();
    wordLabel->setAlignment (Qt::AlignCenter);
    wordLabel->setStyleSheet (QString ("QLabel {"
                                       "font-size: %1px;"
                                       "font-weight: bold;"
                                       "color: #2c3e50;"
                                       "padding: 10px;"
                                       "}")
                                  .arg (Constants::Settings::TITLE_FONT_SIZE));

    headerLayout->addStretch ();
    headerLayout->addWidget (wordLabel);
    headerLayout->addStretch ();

    add2FavoritesButton->setFixedSize (Constants::UI::ICON_SIZE,
                                       Constants::UI::ICON_SIZE);
    headerLayout->addWidget (add2FavoritesButton);

    masterButton->setFixedSize (Constants::UI::ICON_SIZE,
                                Constants::UI::ICON_SIZE);
    headerLayout->addWidget (masterButton);

    separatorLine = new QFrame ();
    separatorLine->setFrameShape (QFrame::HLine);
    separatorLine->setFrameShadow (QFrame::Sunken);
    separatorLine->setStyleSheet ("QFrame {"
                                  "color: #bdc3c7;"
                                  "margin: 10px 0px;"
                                  "}");

    optionAButton = new ElaPushButton (centralWidget);
    optionAButton->setText (tr ("Option A"));
    optionAButton->setMinimumHeight (Constants::UI::DEFAULT_BUTTON_HEIGHT - 10);
    optionAButton->setMaximumHeight (Constants::UI::DEFAULT_BUTTON_HEIGHT - 10);
    optionAButton->setMinimumWidth (150);
    optionAButton->setMaximumWidth (350);
    optionAButton->setBorderRadius (Constants::UI::DEFAULT_BORDER_RADIUS);

    optionBButton = new ElaPushButton (centralWidget);
    optionBButton->setText (tr ("Option B"));
    optionBButton->setMinimumHeight (Constants::UI::DEFAULT_BUTTON_HEIGHT - 10);
    optionBButton->setMaximumHeight (Constants::UI::DEFAULT_BUTTON_HEIGHT - 10);
    optionBButton->setMinimumWidth (150);
    optionBButton->setMaximumWidth (350);
    optionBButton->setBorderRadius (Constants::UI::DEFAULT_BORDER_RADIUS);

    optionCButton = new ElaPushButton (centralWidget);
    optionCButton->setText (tr ("Option C"));
    optionCButton->setMinimumHeight (Constants::UI::DEFAULT_BUTTON_HEIGHT - 10);
    optionCButton->setMaximumHeight (Constants::UI::DEFAULT_BUTTON_HEIGHT - 10);
    optionCButton->setMinimumWidth (150);
    optionCButton->setMaximumWidth (350);
    optionCButton->setBorderRadius (Constants::UI::DEFAULT_BORDER_RADIUS);

    optionDButton = new ElaPushButton (centralWidget);
    optionDButton->setText (tr ("Option D"));
    optionDButton->setMinimumHeight (Constants::UI::DEFAULT_BUTTON_HEIGHT - 10);
    optionDButton->setMaximumHeight (Constants::UI::DEFAULT_BUTTON_HEIGHT - 10);
    optionDButton->setMinimumWidth (150);
    optionDButton->setMaximumWidth (350);
    optionDButton->setBorderRadius (Constants::UI::DEFAULT_BORDER_RADIUS);

    mainLayout->addLayout (headerLayout);
    mainLayout->addWidget (separatorLine);
    mainLayout->addWidget (optionAButton);
    mainLayout->addWidget (optionBButton);
    mainLayout->addWidget (optionCButton);
    mainLayout->addWidget (optionDButton);
    mainLayout->addStretch ();

    addCentralWidget (centralWidget);
}

void QuizCard::initConnections ()
{
    connect (add2FavoritesButton, &ElaIconButton::clicked, this,
             [this] ()
             {
                 // Building...

                 add2FavoritesButton->setAwesome (Constants::UI::UNLIKE_ICON);
                 qDebug () << "Add to Favorites clicked";
             });

    connect (masterButton, &ElaIconButton::clicked, this,
             [this] ()
             {
                 // Building...
                 qDebug () << "Master button clicked";
             });

    connect (optionAButton, &ElaPushButton::clicked, this,
             [this] ()
             {
                 // Building...
                 qDebug () << "Option A clicked";
             });

    connect (optionBButton, &ElaPushButton::clicked, this,
             [this] ()
             {
                 // Building...
                 qDebug () << "Option B clicked";
             });

    connect (optionCButton, &ElaPushButton::clicked, this,
             [this] ()
             {
                 // Building...
                 qDebug () << "Option C clicked";
             });

    connect (optionDButton, &ElaPushButton::clicked, this,
             [this] ()
             {
                 // Building...
                 qDebug () << "Option D clicked";
             });
}

bool QuizCard::isFavorite () const
{

    if (!AccountManager::getInstance ().isLoggedIn ())
    {
        return false;
    }

    QString userId = AccountManager::getInstance ().getUserUuid (
        AccountManager::getInstance ().getUsername ());

    if (userId.isEmpty () || currentWordEntry.word.isEmpty ())
    {
        return false;
    }

    return DbManager::getInstance ().isWordFavorited (userId,
                                                      currentWordEntry.word);
}

void QuizCard::updateAdd2FavoritesButton ()
{
    if (isFavorite ())
    {
        add2FavoritesButton->setAwesome (Constants::UI::LIKE_ICON);
        add2FavoritesButton->setToolTip (tr ("Remove from Favorites"));
    }
    else
    {
        add2FavoritesButton->setAwesome (Constants::UI::UNLIKE_ICON);
        add2FavoritesButton->setToolTip (tr ("Add to Favorites"));
    }
}