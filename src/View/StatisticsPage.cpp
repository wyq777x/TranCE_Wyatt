#include "StatisticsPage.h"
#include "Def.h"
#include "ElaIcon.h"
#include "ElaScrollArea.h"
#include <qboxlayout.h>
#include <qnamespace.h>
#include <qwidget.h>

StatisticsPage::StatisticsPage (QWidget *parent) : TempPage (parent)
{
    setWindowTitle ("Statistics");

    auto *centralWidget = new QWidget (this);
    centralWidget->setWindowTitle ("Statistics");

    QVBoxLayout *statisticsPageLayout = new QVBoxLayout (centralWidget);

    QWidget *statsWidget = new QWidget (centralWidget);
    statsWidget->setStyleSheet ("background-color:rgb(190, 255, 214); "
                                "border-radius: 10px; padding: 10px;");
    statsWidget->setFixedSize (500, 300);

    QVBoxLayout *statsLayout = new QVBoxLayout (statsWidget);
    statsLayout->setAlignment (Qt::AlignTop | Qt::AlignLeft);
    statsLayout->setSpacing (10);
    statsLayout->setContentsMargins (1, 1, 1, 1);

    QLabel *titileLabel = new QLabel ("Statistics");
    titileLabel->setStyleSheet (
        "font-size: 24px; font-weight: normal; color: #333;");
    statsLayout->addWidget (titileLabel);

    QLabel *masteredWordsLabel = new QLabel ();
    masteredWordsLabel->setText (QString ("Mastered Words: %1").arg (0));
    masteredWordsLabel->setStyleSheet (
        "font-size: 16px; font-weight: normal; color: #333;");
    masteredWordsLabel->setAlignment (Qt::AlignLeft);

    statsLayout->addWidget (masteredWordsLabel);

    QLabel *learningWordsLabel = new QLabel ();
    learningWordsLabel->setText (QString ("Learning Words: %1").arg (0));
    learningWordsLabel->setStyleSheet (
        "font-size: 16px; font-weight: normal; color: #333;");
    learningWordsLabel->setAlignment (Qt::AlignLeft);
    statsLayout->addWidget (learningWordsLabel);

    statsLayout->addStretch ();

    ElaIconButton *refreshButton =
        new ElaIconButton (ElaIconType::ArrowRotateLeft);

    refreshButton->setToolTip ("Refresh Statistics");

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

    QLabel *masteredWordsListLabel = new QLabel ("Mastered Words List");
    masteredWordsListLabel->setStyleSheet (
        "font-size: 24px; font-weight: normal; color: #333;");
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