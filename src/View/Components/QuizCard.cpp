#include "QuizCard.h"
#include "Controller/AccountManager.h"
#include "Controller/DbManager.h"
#include "Controller/UIController.h"
#include "Utility/Constants.h"
#include <QTimer>
#include <algorithm>
#include <random>

QuizCard::QuizCard (QWidget *parent) : TempPage (parent)
{
    setWindowTitle (tr ("Quiz Card"));
    setMinimumSize (500, 500);
    setMaximumWidth (700);
    setMaximumHeight (400);
    setSizePolicy (QSizePolicy::Preferred, QSizePolicy::Preferred);
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
    correctAnswerIndex = 0; // The correct answer is always at index 0 initially

    QTimer::singleShot (0, this, [this] () { adjustSize (); });
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
    if (reciteOptions.empty ())
        return;

    QString correctAnswer = reciteOptions[correctAnswerIndex];

    std::random_device rd;
    std::mt19937 g (rd ());

    std::shuffle (reciteOptions.begin (), reciteOptions.end (), g);

    for (int i = 0; i < reciteOptions.size (); ++i)
    {
        if (reciteOptions[i] == correctAnswer)
        {
            correctAnswerIndex = i;
            break;
        }
    }
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

    QTimer::singleShot (0, this,
                        [this] ()
                        {
                            adjustSize ();
                            if (height () > 600)
                            {
                                resize (width (), 600);
                            }
                        });
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
    mainLayout->setSpacing (15);

    headerLayout = new QHBoxLayout ();

    wordLabel = new QLabel ();
    wordLabel->setAlignment (Qt::AlignCenter);
    wordLabel->setWordWrap (true);
    wordLabel->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Preferred);
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
    optionAButton->setMinimumHeight (Constants::UI::DEFAULT_BUTTON_HEIGHT);
    optionAButton->setMinimumWidth (200);
    optionAButton->setSizePolicy (QSizePolicy::Expanding,
                                  QSizePolicy::MinimumExpanding);
    optionAButton->setBorderRadius (Constants::UI::DEFAULT_BORDER_RADIUS);

    optionBButton = new ElaPushButton (centralWidget);
    optionBButton->setText (tr ("Option B"));
    optionBButton->setMinimumHeight (Constants::UI::DEFAULT_BUTTON_HEIGHT);
    optionBButton->setMinimumWidth (200);
    optionBButton->setSizePolicy (QSizePolicy::Expanding,
                                  QSizePolicy::MinimumExpanding);
    optionBButton->setBorderRadius (Constants::UI::DEFAULT_BORDER_RADIUS);

    optionCButton = new ElaPushButton (centralWidget);
    optionCButton->setText (tr ("Option C"));
    optionCButton->setMinimumHeight (Constants::UI::DEFAULT_BUTTON_HEIGHT);
    optionCButton->setMinimumWidth (200);
    optionCButton->setSizePolicy (QSizePolicy::Expanding,
                                  QSizePolicy::MinimumExpanding);
    optionCButton->setBorderRadius (Constants::UI::DEFAULT_BORDER_RADIUS);

    optionDButton = new ElaPushButton (centralWidget);
    optionDButton->setText (tr ("Option D"));
    optionDButton->setMinimumHeight (Constants::UI::DEFAULT_BUTTON_HEIGHT);
    optionDButton->setMinimumWidth (200);
    optionDButton->setSizePolicy (QSizePolicy::Expanding,
                                  QSizePolicy::MinimumExpanding);
    optionDButton->setBorderRadius (Constants::UI::DEFAULT_BORDER_RADIUS);

    mainLayout->addLayout (headerLayout);
    mainLayout->addWidget (separatorLine);
    mainLayout->addWidget (optionAButton);
    mainLayout->addWidget (optionBButton);
    mainLayout->addWidget (optionCButton);
    mainLayout->addWidget (optionDButton);

    addCentralWidget (centralWidget);

    adjustSize ();
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
                 bool isCorrect = validateAnswer (0);
                 showAnswerFeedback (0, isCorrect);
                 emit optionSelected (0, isCorrect);
                 qDebug () << "Option A clicked - "
                           << (isCorrect ? "Correct" : "Wrong");
             });

    connect (optionBButton, &ElaPushButton::clicked, this,
             [this] ()
             {
                 bool isCorrect = validateAnswer (1);
                 showAnswerFeedback (1, isCorrect);
                 emit optionSelected (1, isCorrect);
                 qDebug () << "Option B clicked - "
                           << (isCorrect ? "Correct" : "Wrong");
             });

    connect (optionCButton, &ElaPushButton::clicked, this,
             [this] ()
             {
                 bool isCorrect = validateAnswer (2);
                 showAnswerFeedback (2, isCorrect);
                 emit optionSelected (2, isCorrect);
                 qDebug () << "Option C clicked - "
                           << (isCorrect ? "Correct" : "Wrong");
             });

    connect (optionDButton, &ElaPushButton::clicked, this,
             [this] ()
             {
                 bool isCorrect = validateAnswer (3);
                 showAnswerFeedback (3, isCorrect);
                 emit optionSelected (3, isCorrect);
                 qDebug () << "Option D clicked - "
                           << (isCorrect ? "Correct" : "Wrong");
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
    qDebug () << "Master button clicked";

    // Add to recite history
    if (AccountManager::getInstance ().isLoggedIn ())
    {
        DbManager::getInstance ().addToReciteHistory (
            AccountManager::getInstance ().getUserUuid (
                AccountManager::getInstance ().getUsername ()),
            currentWordEntry.word);
        qDebug () << "Added to recite history:" << currentWordEntry.word;

        // Notify that recite history has been updated
        UIController::getInstance ().notifyReciteHistoryUpdated ();

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
}

bool QuizCard::validateAnswer (int optionIndex) const
{
    return optionIndex == correctAnswerIndex;
}

void QuizCard::showAnswerFeedback (int selectedOptionIndex, bool isCorrect)
{
    std::vector<ElaPushButton *> buttons = {optionAButton, optionBButton,
                                            optionCButton, optionDButton};

    for (auto *button : buttons)
    {
        button->setEnabled (false);
    }

    QString correctStyle = "background-color: #4CAF50;"
                           "color: white;"
                           "border: 2px solid #45a049;";

    QString wrongStyle = "background-color: #f44336;"
                         "color: white;"
                         "border: 2px solid #da190b;";

    buttons[correctAnswerIndex]->setStyleSheet (correctStyle);

    if (!isCorrect && selectedOptionIndex < buttons.size ())
    {
        buttons[selectedOptionIndex]->setStyleSheet (wrongStyle);
    }

    QTimer::singleShot (200, this,
                        [this] ()
                        {
                            resetButtonStates ();
                            close ();
                        });
}

void QuizCard::resetButtonStates ()
{
    // Reset button styles and enable them
    std::vector<ElaPushButton *> buttons = {optionAButton, optionBButton,
                                            optionCButton, optionDButton};

    for (auto *button : buttons)
    {
        button->setStyleSheet (""); // Reset to default style
        button->setEnabled (true);
    }
}