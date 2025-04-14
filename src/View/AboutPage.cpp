#include "AboutPage.h"
#include "ElaFlowLayout.h"
#include <qboxlayout.h>
#include <qgridlayout.h>
#include <qlabel.h>

AboutPage::AboutPage (QWidget *parent) : TempPage (parent)
{
    setWindowTitle ("About");

    auto *centralWidget = new QWidget (this);
    centralWidget->setWindowTitle ("About");
    QVBoxLayout *aboutPageLayout = new QVBoxLayout (centralWidget);
    aboutPageLayout->setAlignment (Qt::AlignHCenter | Qt::AlignTop);
    aboutPageLayout->setSpacing (20);

    QHBoxLayout *titleLayout = new QHBoxLayout (centralWidget);
    titleLayout->setAlignment (Qt::AlignHCenter);
    titleLayout->setSpacing (5);

    QLabel *logoLabel = new QLabel (centralWidget);
    logoLabel->setPixmap (QPixmap (":/res/image/learnENG.ico"));
    logoLabel->setFixedSize (50, 50);
    logoLabel->setScaledContents (true);

    titleLayout->addWidget (logoLabel);

    QLabel *titleLabel = new QLabel ("TranCE_Wyatt", centralWidget);
    titleLabel->setStyleSheet (
        "font-size: 24px; font-weight: bold; color: #333;");
    titleLabel->setFont (QFont ("Noto Sans", 24, QFont::Bold));
    titleLabel->setFixedHeight (50);

    titleLayout->addWidget (titleLabel);

    aboutPageLayout->addLayout (titleLayout);
    addCentralWidget (centralWidget, true, true, 0);
}