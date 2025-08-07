#include "QuizCard.h"
#include "Controller/AccountManager.h"
#include "Controller/DbManager.h"
#include "Utility/Constants.h"
#include <algorithm>
#include <random>

QuizCard::QuizCard (QWidget *parent) : TempPage (parent)
{
    setWindowTitle (tr ("Quiz Card"));
    setFixedSize (400, 400);
    setStyleSheet (
        "background-color: #f0f0f0; font-family: 'Noto Sans', sans-serif;");

    initUI ();
    initConnections ();
}

void QuizCard::setAdd2FavoritesButtonEnabled (bool enabled)
{
    add2FavoritesButton->setEnabled (enabled);
    add2FavoritesButton->setVisible (enabled);
}

void QuizCard::setWordEntry (WordEntry &entry)
{
    reciteOptions.clear ();

    currentWordEntry = entry;

    wordLabel->setText (entry.word);

    updateAdd2FavoritesButton ();

    reciteOptions.emplace_back (entry.translation);
}

void QuizCard::fillReciteOptions ()
{
    std::vector<QString> wrongTranslations =
        DbManager::getInstance ().getRandomWrongTranslations (
            currentWordEntry.translation, 3);

    reciteOptions.insert (reciteOptions.end (), wrongTranslations.begin (),
                          wrongTranslations.end ());
}

void QuizCard::shuffleReciteOptions ()
{
    std::random_device rd;
    std::mt19937 g (rd ());

    std::shuffle (reciteOptions.begin (), reciteOptions.end (), g);
}

void QuizCard::setReciteOptions (const std::vector<QString> &options)
{
    if (options.size () < 4)
    {
        qWarning () << "Not enough options to set in QuizCard";
        return;
    }

    optionAButton->setText (options[0]);
    optionBButton->setText (options[1]);
    optionCButton->setText (options[2]);
    optionDButton->setText (options[3]);

    optionAButton->setEnabled (true);
    optionBButton->setEnabled (true);
    optionCButton->setEnabled (true);
    optionDButton->setEnabled (true);
}

void QuizCard::initUI ()
{
    // UI Layout:

    //   ENGLISH WORD     (ADD2FAVORITES_BUTTON) MASTER_BUTTON
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
             &QuizCard::onAdd2FavoritesClicked);

    connect (masterButton, &ElaIconButton::clicked, this,
             &QuizCard::onMasterButtonClicked);

    connect (optionAButton, &ElaPushButton::clicked, this,
             [this] ()
             {
                 // Building...
                 emit optionSelected (0);
                 qDebug () << "Option A clicked";
             });

    connect (optionBButton, &ElaPushButton::clicked, this,
             [this] ()
             {
                 // Building...
                 emit optionSelected (1);
                 qDebug () << "Option B clicked";
             });

    connect (optionCButton, &ElaPushButton::clicked, this,
             [this] ()
             {
                 // Building...
                 emit optionSelected (2);
                 qDebug () << "Option C clicked";
             });

    connect (optionDButton, &ElaPushButton::clicked, this,
             [this] ()
             {
                 // Building...
                 emit optionSelected (3);
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

void QuizCard::addToUserFavorites (const QString &userId, const QString &word)
{
    DbManager::getInstance ().addToUserFavorites (userId, word);
}

void QuizCard::removeFromUserFavorites (const QString &userId,
                                        const QString &word)
{
    DbManager::getInstance ().removeFromUserFavorites (userId, word);
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

void QuizCard::onAdd2FavoritesClicked ()
{
    if (!AccountManager::getInstance ().isLoggedIn ())
    {
        qDebug () << "User not logged in, cannot modify favorites";
        return;
    }

    QString userId = AccountManager::getInstance ().getUserUuid (
        AccountManager::getInstance ().getUsername ());

    if (userId.isEmpty () || currentWordEntry.word.isEmpty ())
    {
        qDebug () << "Invalid user ID or word, cannot modify favorites";
        return;
    }

    if (isFavorite ())
    {
        removeFromUserFavorites (userId, currentWordEntry.word);
        qDebug () << "Removed word from favorites:" << currentWordEntry.word;
    }
    else
    {
        addToUserFavorites (userId, currentWordEntry.word);
        qDebug () << "Added word to favorites:" << currentWordEntry.word;
    }

    // Update button state and tooltip
    updateAdd2FavoritesButton ();
}

void QuizCard::onMasterButtonClicked ()
{
    // showNextQuizCard by RecitePage
    emit masterButtonClicked ();

    // Add to recite history
    DbManager::getInstance ().addToReciteHistory (
        AccountManager::getInstance ().getUserUuid (
            AccountManager::getInstance ().getUsername ()),
        currentWordEntry.word);
    qDebug () << "Added to recite history:" << currentWordEntry.word;

    // Add to mastered list
    auto result = DbManager::getInstance ().updateWordStatus (
        AccountManager::getInstance ().getUserUuid (
            AccountManager::getInstance ().getUsername ()),
        currentWordEntry.word, 1);
    if (result != ChangeResult::Success)
    {
        qWarning () << "Failed to update word status in database:"
                    << getErrorMessage (result, ChangeResultMessage);
        return;
    }

    qDebug () << currentWordEntry.word << " mastered.";
}