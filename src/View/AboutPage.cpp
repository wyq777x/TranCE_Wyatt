#include "AboutPage.h"

#include "ElaPushButton.h"
#include "ElaScrollArea.h"
#include "ElaWidget.h"
#include "Utility/Constants.h"
#include <qboxlayout.h>
#include <qdesktopservices.h>
#include <qgridlayout.h>
#include <qlabel.h>
#include <qobject.h>


AboutPage::AboutPage (QWidget *parent) : TempPage (parent)
{
    setWindowTitle (tr ("About"));

    auto *centralWidget = new QWidget (this);
    centralWidget->setWindowTitle (tr ("About"));
    QVBoxLayout *aboutPageLayout = new QVBoxLayout (centralWidget);
    aboutPageLayout->setAlignment (Qt::AlignHCenter | Qt::AlignTop);
    aboutPageLayout->setSpacing (20);
    aboutPageLayout->setContentsMargins (5, 20, 5, 20);

    QHBoxLayout *titleLayout = new QHBoxLayout (centralWidget);
    titleLayout->setAlignment (Qt::AlignHCenter);
    titleLayout->setSpacing (5);

    QLabel *logoLabel = new QLabel (centralWidget);
    logoLabel->setPixmap (QPixmap (Constants::Resources::APP_ICON));
    logoLabel->setFixedSize (50, 50);
    logoLabel->setScaledContents (true);

    titleLayout->addWidget (logoLabel);

    QLabel *titleLabel = new QLabel (tr ("TranCE_Wyatt"), centralWidget);
    titleLabel->setStyleSheet (
        QString ("font-size: %1px; font-weight: bold; color: #333;")
            .arg (Constants::Settings::TITLE_FONT_SIZE));
    titleLabel->setFont (QFont (Constants::Settings::DEFAULT_FONT_FAMILY,
                                Constants::Settings::TITLE_FONT_SIZE,
                                QFont::Bold));
    titleLabel->setFixedHeight (50);

    titleLayout->addWidget (titleLabel);

    aboutPageLayout->addLayout (titleLayout);

    QLabel *descriptionLabel = new QLabel (
        tr ("TranCE_Wyatt is a tool for learning English vocabulary.\nIt "
            "provides a simple and efficient way to learn and memorize English "
            "words."),
        centralWidget);
    descriptionLabel->setStyleSheet (
        QString ("font-size: %1px; color: #333;")
            .arg (Constants::Settings::DEFAULT_FONT_SIZE));
    descriptionLabel->setFont (QFont (Constants::Settings::DEFAULT_FONT_FAMILY,
                                      Constants::Settings::DEFAULT_FONT_SIZE,
                                      QFont::Normal));
    descriptionLabel->setWordWrap (true);
    descriptionLabel->setAlignment (Qt::AlignHCenter);
    aboutPageLayout->addWidget (descriptionLabel);

    QVBoxLayout *detailsLayout = new QVBoxLayout (centralWidget);
    detailsLayout->setAlignment (Qt::AlignVCenter);
    detailsLayout->setSpacing (2);
    detailsLayout->setContentsMargins (5, 0, 5, 0);

    ElaPushButton *repoButton =
        new ElaPushButton (tr ("GitHub Repository"), centralWidget);
    repoButton->setMinimumWidth (600);
    repoButton->setMaximumWidth (600);
    connect (
        repoButton, &ElaPushButton::clicked, this, [] ()
        { QDesktopServices::openUrl (QUrl (Constants::Urls::GITHUB_REPO)); });

    detailsLayout->addWidget (repoButton);

    ElaPushButton *licenseButton =
        new ElaPushButton (tr ("License"), centralWidget);
    licenseButton->setMinimumWidth (600);
    licenseButton->setMaximumWidth (600);
    connect (
        licenseButton, &ElaPushButton::clicked, this,
        [] ()
        {
            ElaWidget *licensePage = new ElaWidget ();

            licensePage->setWindowTitle (tr ("MIT License"));
            licensePage->setMinimumSize (800, 600);
            licensePage->setMaximumSize (800, 600);
            licensePage->setStyleSheet ("QWidget { background-color: "
                                        "transparent; border-radius: 10px;}");

            QVBoxLayout *licenseLayout = new QVBoxLayout (licensePage);
            licenseLayout->setAlignment (Qt::AlignHCenter | Qt::AlignTop);
            licenseLayout->setSpacing (20);
            licenseLayout->setContentsMargins (15, 40, 15, 40);

            QLabel *licenseLabel = new QLabel (licensePage);
            licenseLabel->setText (tr ("MIT License"));
            licenseLabel->setAlignment (Qt::AlignHCenter);
            licenseLabel->setStyleSheet (
                QString ("font-size: %1px; font-weight: bold; color: #333;")
                    .arg (Constants::Settings::DEFAULT_FONT_SIZE));
            licenseLabel->setFont (
                QFont (Constants::Settings::DEFAULT_FONT_FAMILY,
                       Constants::Settings::DEFAULT_FONT_SIZE, QFont::Bold));
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
            licenseTextLabel->setFont (
                QFont (Constants::Settings::DEFAULT_FONT_FAMILY,
                       Constants::Settings::DEFAULT_FONT_SIZE, QFont::Normal));
            licenseTextLabel->setWordWrap (true);

            ElaScrollArea *licenseTextScrollArea =
                new ElaScrollArea (licensePage);
            licenseTextScrollArea->setWidget (licenseTextLabel);
            licenseTextScrollArea->setWidgetResizable (true);

            licenseLayout->addWidget (licenseTextScrollArea);
            licensePage->setLayout (licenseLayout);
            licensePage->show ();
        });
    detailsLayout->addWidget (licenseButton);

    aboutPageLayout->addLayout (detailsLayout);
    addCentralWidget (centralWidget, true, true, 0);
}