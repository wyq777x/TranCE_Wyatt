#include "AboutPage.h"
#include "ElaFlowLayout.h"
#include "ElaPushButton.h"
#include "ElaScrollArea.h"
#include "ElaWidget.h"
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

    ElaPushButton *licenseButton = new ElaPushButton ("License", centralWidget);
    licenseButton->setMinimumWidth (600);
    licenseButton->setMaximumWidth (600);
    connect (
        licenseButton, &ElaPushButton::clicked, this,
        [] ()
        {
            ElaWidget *licensePage = new ElaWidget ();
            licensePage->setWindowTitle ("MIT License");
            licensePage->setMinimumSize (800, 600);
            licensePage->setMaximumSize (800, 600);
            licensePage->setStyleSheet ("QWidget { background-color: "
                                        "transparent; border-radius: 10px;}");

            QVBoxLayout *licenseLayout = new QVBoxLayout (licensePage);
            licenseLayout->setAlignment (Qt::AlignHCenter | Qt::AlignTop);
            licenseLayout->setSpacing (20);
            licenseLayout->setContentsMargins (15, 40, 15, 40);

            QLabel *licenseLabel = new QLabel (licensePage);
            licenseLabel->setText ("MIT License");
            licenseLabel->setAlignment (Qt::AlignHCenter);
            licenseLabel->setStyleSheet (
                "font-size: 16px; font-weight: bold; color: #333;");
            licenseLabel->setFont (QFont ("Noto Sans", 16, QFont::Bold));
            licenseLabel->setWordWrap (true);
            licenseLayout->addWidget (licenseLabel);
            QLabel *licenseTextLabel = new QLabel (licensePage);
            licenseTextLabel->setText (
                "Copyright (c) 2025 wyq777x/Wyatt Wong\n\n"
                "Permission is hereby granted, free of charge, to any person "
                "obtaining a copy\n"
                "of this software and associated documentation files (the "
                "\"Software\"), to deal\n"
                "in the Software without restriction, including without "
                "limitation the rights\n"
                "to use, copy, modify, merge, publish, distribute, sublicense, "
                "and/or sell\n"
                "copies of the Software, and to permit persons to whom the "
                "Software is\n"
                "furnished to do so, subject to the following conditions:\n\n"
                "The above copyright notice and this permission notice shall "
                "be included in all\n"
                "copies or substantial portions of the Software.\n\n"
                "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY "
                "KIND, EXPRESS OR\n"
                "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF "
                "MERCHANTABILITY,\n"
                "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO "
                "EVENT SHALL THE\n"
                "AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES "
                "OR OTHER\n"
                "LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR "
                "OTHERWISE, ARISING FROM,\n"
                "OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER "
                "DEALINGS IN THE\n"
                "SOFTWARE.");
            licenseTextLabel->setAlignment (Qt::AlignHCenter);
            licenseTextLabel->setStyleSheet ("font-size: 16px; color: #333;");
            licenseTextLabel->setFont (QFont ("Noto Sans", 16, QFont::Normal));
            licenseTextLabel->setWordWrap (true);
            licenseLayout->addWidget (licenseTextLabel);
            licensePage->setLayout (licenseLayout);
            licensePage->show ();
        });
    detailsLayout->addWidget (licenseButton);

    aboutPageLayout->addLayout (detailsLayout);
    addCentralWidget (centralWidget, true, true, 0);
}