#include "StatisticsPage.h"
#include "Controller/AccountManager.h"
#include "Controller/DbManager.h"
#include "Utility/Constants.h"

StatisticsPage::StatisticsPage (QWidget *parent) : TempPage (parent)
{
    setWindowTitle (tr ("Statistics"));

    initUI ();
    initConnections ();
    refreshStatistics ();
}

void StatisticsPage::initUI ()
{
    centralWidget = new QWidget (this);
    centralWidget->setWindowTitle (tr ("Statistics"));

    statisticsPageLayout = new QVBoxLayout (centralWidget);

    statsWidget = new QWidget (centralWidget);
    statsWidget->setStyleSheet ("background-color:rgb(190, 255, 214); "
                                "border-radius: 10px; padding: 10px;");
    statsWidget->setFixedSize (600, 500);

    statsLayout = new QVBoxLayout (statsWidget);
    statsLayout->setAlignment (Qt::AlignTop | Qt::AlignLeft);
    statsLayout->setSpacing (10);
    statsLayout->setContentsMargins (1, 1, 1, 1);

    titileLabel = new QLabel (tr ("Statistics"));
    titileLabel->setStyleSheet (
        QString ("font-size: %1px; font-weight: normal; color: #333;")
            .arg (Constants::Settings::TITLE_FONT_SIZE));
    statsLayout->addWidget (titileLabel);

    masteredWordsLabel = new QLabel ();
    masteredWordsLabel->setText (tr ("Mastered Words: %1").arg (0));
    masteredWordsLabel->setStyleSheet (
        QString ("font-size: %1px; font-weight: normal; color: #333;")
            .arg (Constants::Settings::DEFAULT_FONT_SIZE));
    masteredWordsLabel->setAlignment (Qt::AlignLeft);
    statsLayout->addWidget (masteredWordsLabel);

    learningWordsLabel = new QLabel ();
    learningWordsLabel->setText (tr ("Learning Words: %1").arg (0));
    learningWordsLabel->setStyleSheet (
        QString ("font-size: %1px; font-weight: normal; color: #333;")
            .arg (Constants::Settings::DEFAULT_FONT_SIZE));
    learningWordsLabel->setAlignment (Qt::AlignLeft);
    statsLayout->addWidget (learningWordsLabel);

    statsLayout->addStretch ();

    refreshButton = new ElaIconButton (ElaIconType::ArrowRotateLeft);
    refreshButton->setToolTip (tr ("Refresh Statistics"));
    refreshButton->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Fixed);
    refreshButton->setMinimumHeight (40);
    statsLayout->addWidget (refreshButton);

    statisticsPageLayout->addWidget (statsWidget);

    statisticsPageLayout->setSpacing (15);
    statisticsPageLayout->setAlignment (Qt::AlignCenter | Qt::AlignTop);

    addCentralWidget (centralWidget, true, true, 0);
}

void StatisticsPage::initConnections ()
{

    connect (&AccountManager::getInstance (), &AccountManager::loginSuccessful,
             this, &StatisticsPage::refreshStatistics);
    connect (&AccountManager::getInstance (), &AccountManager::logoutSuccessful,
             this, &StatisticsPage::refreshStatistics);
    connect (refreshButton, &ElaIconButton::clicked, this,
             &StatisticsPage::onRefreshButtonClicked);
}

void StatisticsPage::refreshStatistics ()
{

    auto &accountManager = AccountManager::getInstance ();
    QString currentUsername = accountManager.getUsername ();

    if (currentUsername.isEmpty ())
    {

        masteredWordsLabel->setText (tr ("Mastered Words: %1").arg (0));
        learningWordsLabel->setText (tr ("Learning Words: %1").arg (0));
        return;
    }

    auto &dbManager = DbManager::getInstance ();
    auto userId = dbManager.getUserId (currentUsername);

    if (!userId.has_value ())
    {

        masteredWordsLabel->setText (tr ("Mastered Words: %1").arg (0));
        learningWordsLabel->setText (tr ("Learning Words: %1").arg (0));
        return;
    }

    int masteredCount = dbManager.getMasteredWordsCount (userId.value ());
    int learningCount = dbManager.getLearningWordsCount (userId.value ());

    masteredWordsLabel->setText (tr ("Mastered Words: %1").arg (masteredCount));
    learningWordsLabel->setText (tr ("Learning Words: %1").arg (learningCount));

    qDebug () << "Statistics refreshed. ";
}

void StatisticsPage::onRefreshButtonClicked () { refreshStatistics (); }