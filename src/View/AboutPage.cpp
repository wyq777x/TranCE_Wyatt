#include "AboutPage.h"
#include "ElaFlowLayout.h"
#include "ElaPushButton.h"
#include <qboxlayout.h>
#include <qdesktopservices.h>
#include <qgridlayout.h>
#include <qlabel.h>
#include <qobject.h>

AboutPage::AboutPage (QWidget *parent) : TempPage (parent)
{
    setWindowTitle ("About");

    auto *centralWidget = new QWidget (this);
    centralWidget->setWindowTitle ("About");
    QVBoxLayout *aboutPageLayout = new QVBoxLayout (centralWidget);
    aboutPageLayout->setAlignment (Qt::AlignHCenter | Qt::AlignTop);
    aboutPageLayout->setSpacing (20);
    aboutPageLayout->setContentsMargins (5, 20, 5, 20);

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

    QLabel *descriptionLabel = new QLabel (
        R"(TranCE_Wyatt is a tool for learning English vocabulary.
It provides a simple and efficient way to learn and memorize English words.)",
        centralWidget);
    descriptionLabel->setStyleSheet ("font-size: 16px; color: #333;");
    descriptionLabel->setFont (QFont ("Noto Sans", 16, QFont::Normal));
    descriptionLabel->setWordWrap (true);
    descriptionLabel->setAlignment (Qt::AlignHCenter);
    aboutPageLayout->addWidget (descriptionLabel);

    QVBoxLayout *detailsLayout = new QVBoxLayout (centralWidget);
    detailsLayout->setAlignment (Qt::AlignVCenter);
    detailsLayout->setSpacing (2);
    detailsLayout->setContentsMargins (5, 0, 5, 0);

    ElaPushButton *repoButton =
        new ElaPushButton ("GitHub Repository", centralWidget);
    repoButton->setMinimumWidth (600);
    repoButton->setMaximumWidth (600);
    connect (repoButton, &ElaPushButton::clicked, this,
             [] ()
             {
                 QDesktopServices::openUrl (
                     QUrl ("https://github.com/wyq777x/TranCE_Wyatt"));
             });

    detailsLayout->addWidget (repoButton);
    aboutPageLayout->addLayout (detailsLayout);

    addCentralWidget (centralWidget, true, true, 0);
}