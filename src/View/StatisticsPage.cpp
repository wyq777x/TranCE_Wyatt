#include "StatisticsPage.h"
#include "Def.h"
#include "ElaIcon.h"
#include "ElaScrollArea.h"
#include "Utility/Constants.h"

StatisticsPage::StatisticsPage (QWidget *parent) : TempPage (parent)
{
    setWindowTitle (tr ("Statistics"));

    auto *centralWidget = new QWidget (this);
    centralWidget->setWindowTitle (tr ("Statistics"));

    QVBoxLayout *statisticsPageLayout = new QVBoxLayout (centralWidget);

    QWidget *statsWidget = new QWidget (centralWidget);
    statsWidget->setStyleSheet ("background-color:rgb(190, 255, 214); "
                                "border-radius: 10px; padding: 10px;");
    statsWidget->setFixedSize (500, 300);

    QVBoxLayout *statsLayout = new QVBoxLayout (statsWidget);
    statsLayout->setAlignment (Qt::AlignTop | Qt::AlignLeft);
    statsLayout->setSpacing (10);
    statsLayout->setContentsMargins (1, 1, 1, 1);

    QLabel *titileLabel = new QLabel (tr ("Statistics"));
    titileLabel->setStyleSheet (
        QString ("font-size: %1px; font-weight: normal; color: #333;")
            .arg (Constants::Settings::TITLE_FONT_SIZE));
    statsLayout->addWidget (titileLabel);

    QLabel *masteredWordsLabel = new QLabel ();
    masteredWordsLabel->setText (tr ("Mastered Words: %1").arg (0));
    masteredWordsLabel->setStyleSheet (
        QString ("font-size: %1px; font-weight: normal; color: #333;")
            .arg (Constants::Settings::DEFAULT_FONT_SIZE));
    masteredWordsLabel->setAlignment (Qt::AlignLeft);

    statsLayout->addWidget (masteredWordsLabel);

    QLabel *learningWordsLabel = new QLabel ();
    learningWordsLabel->setText (tr ("Learning Words: %1").arg (0));
    learningWordsLabel->setStyleSheet (
        QString ("font-size: %1px; font-weight: normal; color: #333;")
            .arg (Constants::Settings::DEFAULT_FONT_SIZE));
    learningWordsLabel->setAlignment (Qt::AlignLeft);
    statsLayout->addWidget (learningWordsLabel);

    statsLayout->addStretch ();

    ElaIconButton *refreshButton =
        new ElaIconButton (ElaIconType::ArrowRotateLeft);

    refreshButton->setToolTip (tr ("Refresh Statistics"));

    refreshButton->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Fixed);
    refreshButton->setMinimumHeight (40);
    statsLayout->addWidget (refreshButton);

    statisticsPageLayout->addWidget (statsWidget);

    QWidget *masteredWordsWidget = new QWidget (centralWidget);
    masteredWordsWidget->setStyleSheet ("background-color:rgb(249, 203, 52); "
                                        "border-radius: 10px; padding: 10px;");
    masteredWordsWidget->setFixedSize (500, 300);

    QVBoxLayout *masteredWordsLayout = new QVBoxLayout (masteredWordsWidget);
    masteredWordsLayout->setAlignment (Qt::AlignTop | Qt::AlignLeft);
    masteredWordsLayout->setSpacing (10);
    masteredWordsLayout->setContentsMargins (1, 1, 1, 1);

    QLabel *masteredWordsListLabel = new QLabel (tr ("Mastered Words List"));
    masteredWordsListLabel->setStyleSheet (
        QString ("font-size: %1px; font-weight: normal; color: #333;")
            .arg (Constants::Settings::TITLE_FONT_SIZE));
    masteredWordsLayout->addWidget (masteredWordsListLabel);
    masteredWordsListLabel->setAlignment (Qt::AlignLeft);

    ElaScrollArea *masteredWordsScrollArea = new ElaScrollArea ();

    ElaListView *masteredWordsList = new ElaListView (masteredWordsScrollArea);
    masteredWordsList->setStyleSheet (
        "background-color: rgb(255, 255, 255); border-radius: 10px;");

    masteredWordsList->setFixedSize (500, 200);

    masteredWordsLayout->addWidget (masteredWordsScrollArea);
    masteredWordsScrollArea->setWidget (masteredWordsList);
    masteredWordsScrollArea->setWidgetResizable (true);

    statisticsPageLayout->addWidget (masteredWordsWidget);

    statisticsPageLayout->setSpacing (15);
    statisticsPageLayout->setAlignment (Qt::AlignCenter | Qt::AlignTop);

    addCentralWidget (centralWidget, true, true, 0);
}