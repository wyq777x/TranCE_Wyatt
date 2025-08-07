#include "RecitePage.h"
#include "Controller/AccountManager.h"
#include "Controller/DbManager.h"
#include "Controller/UIController.h"
#include "Utility/ClickableWidget.h"
#include "Utility/Constants.h"
#include "Utility/Result.h"
#include "View/Components/QuizCard.h"
#include <QMessageBox>

RecitePage::RecitePage (QWidget *parent) : TempPage (parent)
{
    setWindowTitle (tr ("Recite Page"));

    initUI ();
    initConnections ();
    initializeCardAmount ();
}

void RecitePage::onLoginSuccessful ()
{
    // Building ...

    std::pair<int, int> progress = DbManager::getInstance ().getProgress (
        AccountManager::getInstance ().getUserUuid (
            AccountManager::getInstance ().getUsername ()));

    setProgress (progress.first, progress.second);

    currentCardIndex = currentProgress % totalProgress;

    if (currentCardIndex >= totalProgress)
    {
        currentCardIndex = 0;
    }

    // update Favorites and Mastered widgets
}

void RecitePage::onLogoutSuccessful ()
{
    // Building ...
    setProgress (0, 15);

    // reset Favorites and Mastered widgets
}

void RecitePage::onReciteButtonClicked ()
{
    showNextQuizCard ();

    qDebug () << "Recite button clicked";
}

void RecitePage::updateProgressUI (int current, int total)
{
    if (progressLabel)
    {
        progressLabel->setText (
            tr ("Progress: %1/%2").arg (current).arg (total));
    }
}

void RecitePage::initUI ()
{
    centralWidget = new QWidget (this);
    centralWidget->setWindowTitle (tr ("Recite"));

    recitePageLayout = new QVBoxLayout (centralWidget);
    recitePageLayout->setAlignment (Qt::AlignHCenter | Qt::AlignTop);
    recitePageLayout->setSizeConstraint (QLayout::SetMinimumSize);

    containerWidget = new QWidget (centralWidget);
    containerWidget->setFixedSize (600, 200);
    containerLayout = new QGridLayout (containerWidget);
    containerLayout->setContentsMargins (0, 0, 0, 0);
    containerLayout->setSpacing (0);

    reciteBgWidget = new QWidget ();
    QGraphicsBlurEffect *blurEffect = new QGraphicsBlurEffect (reciteBgWidget);
    blurEffect->setBlurRadius (5);
    reciteBgWidget->setGraphicsEffect (blurEffect);
    reciteBgWidget->setStyleSheet (
        "QWidget { background-color: rgba(175, 238, 255, 0.8); "
        "border-radius: 10px; }");
    reciteBgWidget->setFixedSize (600, 200);

    reciteContentWidget = new QWidget ();
    reciteContentWidget->setStyleSheet ("background-color: transparent;");
    reciteWidgetLayout = new QVBoxLayout (reciteContentWidget);
    reciteWidgetLayout->setAlignment (Qt::AlignCenter);

    progressLabel = new QLabel (reciteContentWidget);
    progressLabel->setText (tr ("Progress: %1/%2")
                                .arg (getProgress ().first)
                                .arg (getProgress ().second));
    progressLabel->setAlignment (Qt::AlignCenter);
    progressLabel->setStyleSheet (
        QString ("font-size: %1px; font-weight: bold; color: #333;")
            .arg (Constants::Settings::SUBTITLE_FONT_SIZE));
    progressLabel->setFont (QFont (Constants::Settings::DEFAULT_FONT_FAMILY,
                                   Constants::Settings::DEFAULT_FONT_SIZE,
                                   QFont::Normal));
    reciteWidgetLayout->addWidget (progressLabel);

    reciteButton =
        new ElaPushButton (tr ("Start Reciting"), reciteContentWidget);
    reciteButton->setFixedSize (180, 40);
    reciteButton->setStyleSheet (
        "QPushButton { background-color: #4CAF50; color: white; "
        "border-radius: 5px; }"
        "QPushButton:hover { background-color: #45a049; }");
    reciteButton->setFont (
        QFont (Constants::Settings::DEFAULT_FONT_FAMILY, 12, QFont::Normal));
    reciteWidgetLayout->addWidget (reciteButton);

    containerLayout->addWidget (reciteBgWidget, 0, 0);
    containerLayout->addWidget (reciteContentWidget, 0, 0);

    recitePageLayout->addWidget (containerWidget);

    HBoxContainer = new ClickableWidget (centralWidget);
    HBoxLayout = new QHBoxLayout (HBoxContainer);

    HBoxLayout->setAlignment (Qt::AlignCenter);
    HBoxLayout->setSpacing (20);

    favoritesWidget = new ClickableWidget (HBoxContainer);

    favoritesWidget->setStyleSheet (
        "background-color: #FFF8DC; border-radius: 8px;");
    favoritesWidget->setFixedSize (300, 300);
    favoritesWidget->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);

    favoritesContentWidget = new QWidget (favoritesWidget);
    favoritesContentWidget->setStyleSheet (
        "background-color: #FFF8DC; border-radius: 8px;");

    favoritesLabel = new QLabel ();
    favoritesLabel->setText (tr ("Favorites"));
    favoritesLabel->setStyleSheet (
        QString ("font-size: %1px; color: #333;")
            .arg (Constants::Settings::SUBTITLE_FONT_SIZE));

    favoritesLayout = new QVBoxLayout (favoritesContentWidget);
    favoritesLayout->addWidget (favoritesLabel);
    favoritesLayout->addStretch ();

    favoritesWidgetLayout = new QHBoxLayout (favoritesWidget);
    favoritesWidgetLayout->setContentsMargins (0, 0, 0, 0);
    favoritesWidgetLayout->addWidget (favoritesContentWidget);

    masteredWidget = new ClickableWidget (HBoxContainer);

    masteredWidget->setStyleSheet (
        "background-color: #E0FFE0; border-radius: 8px;");
    masteredWidget->setFixedSize (300, 300);
    masteredWidget->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);

    masteredContentWidget = new QWidget (masteredWidget);
    masteredContentWidget->setStyleSheet (
        "background-color: #E0FFE0; border-radius: 8px;");

    masteredLabel = new QLabel ();
    masteredLabel->setText (tr ("Mastered"));
    masteredLabel->setStyleSheet (
        QString ("font-size: %1px; color: #333;")
            .arg (Constants::Settings::SUBTITLE_FONT_SIZE));

    masteredLayout = new QVBoxLayout (masteredContentWidget);
    masteredLayout->addWidget (masteredLabel);
    masteredLayout->addStretch ();

    masteredWidgetLayout = new QHBoxLayout (masteredWidget);
    masteredWidgetLayout->setContentsMargins (0, 0, 0, 0);
    masteredWidgetLayout->addWidget (masteredContentWidget);

    HBoxLayout->addWidget (favoritesWidget);
    HBoxLayout->addWidget (masteredWidget);

    HBoxContainer->setMinimumSize (620, 300);

    recitePageLayout->addWidget (HBoxContainer);
    addCentralWidget (centralWidget, true, true, 0);
}

void RecitePage::initConnections ()
{

    connect (&AccountManager::getInstance (), &AccountManager::loginSuccessful,
             this, &RecitePage::onLoginSuccessful);

    connect (&AccountManager::getInstance (), &AccountManager::logoutSuccessful,
             this, &RecitePage::onLogoutSuccessful);

    connect (this, &RecitePage::progressUpdated, this,
             &RecitePage::updateProgressUI);

    connect (
        this, &RecitePage::progressUpdated, this,
        [this] (int current, int total)
        {
            // Building...
            if (AccountManager::getInstance ().isLoggedIn ())
            {
                QString userId = AccountManager::getInstance ().getUserUuid (
                    AccountManager::getInstance ().getUsername ());
                if (!userId.isEmpty ())
                {

                    auto result =
                        DbManager::getInstance ().updateReciteProgress (
                            current, total, userId);

                    if (result != ChangeResult::Success)
                    {
                        qWarning ()
                            << "Failed to update progress in database:"
                            << getErrorMessage (result, ChangeResultMessage);

                        return;
                    }

                    qDebug () << "Progress updated in database:" << current
                              << "/" << total;
                }
            }
        });

    connect (reciteButton, &ElaPushButton::clicked, this,
             &RecitePage::onReciteButtonClicked);

    connect (favoritesWidget, &ClickableWidget::clicked, this,
             [] () { qDebug () << "Favorites widget clicked"; });

    connect (masteredWidget, &ClickableWidget::clicked, this,
             [] () { qDebug () << "Mastered widget clicked"; });
}

void RecitePage::initializeCardAmount ()
{
    // Building...
    Card_amount.clear ();
    Card_amount.reserve (totalProgress);

    for (int i = 0; i < totalProgress; ++i)
    {
        WordEntry entry;
        entry.word = QString ("Word_%1").arg (i + 1);
        entry.translation = QString ("Translation_%1").arg (i + 1);
        Card_amount.emplace_back (entry);
    }
}

void RecitePage::showNextQuizCard ()
{
    if (currentCardIndex >= Card_amount.size ())
    {
        showCompletionDialog ();

        if (quizCard)
        {
            quizCard->close ();
        }

        return;
    }

    quizCard = UIController::getInstance ().showQuizCard (
        Card_amount[currentCardIndex]);

    connect (quizCard, &QuizCard::optionSelected, this,
             &RecitePage::handleQuizCardOptionSelected, Qt::UniqueConnection);

    connect (quizCard, &QuizCard::masterButtonClicked, this,
             &RecitePage::handleQuizCardOptionSelected, Qt::UniqueConnection);
}

void RecitePage::handleQuizCardOptionSelected ()
{
    currentCardIndex++;
    setProgress (currentCardIndex, totalProgress);

    if (currentCardIndex >= totalProgress)
    {
        showCompletionDialog ();
        if (quizCard)
        {
            quizCard->close ();
        }
    }
    else
    {
        showNextQuizCard ();
    }
}

void RecitePage::showCompletionDialog ()
{
    QMessageBox::information (
        this, tr ("Recitation Complete"),
        tr ("Congratulations! You have completed one round of recitation."));

    // Reset progress to initial state
    setProgress (0, totalProgress);

    currentCardIndex = 0;
}