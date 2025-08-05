#include "StatisticsPage.h"
#include "Utility/Constants.h"

StatisticsPage::StatisticsPage (QWidget *parent) : TempPage (parent)
{
    setWindowTitle (tr ("Statistics"));

    initUI ();
    initConnections ();
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
    // Add connections here when needed
    // For example:
    // connect(refreshButton, &ElaIconButton::clicked, this,
    // &StatisticsPage::onRefreshButtonClicked);
}