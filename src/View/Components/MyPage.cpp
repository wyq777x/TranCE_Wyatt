#include "MyPage.h"
#include "Controller/AccountManager.h"
#include "ElaLineEdit.h"
#include "ElaPushButton.h"
#include "Utility/Constants.h"
#include "Utility/Result.h"
#include <qnamespace.h>

MyPage::MyPage (QWidget *parent) : TempPage (parent)
{
    setWindowTitle (tr ("My Page"));
    setMinimumSize (800, 600);
    setMaximumSize (800, 600);
    setStyleSheet (
        "background-color: #f0f0f0; font-family: 'Noto Sans', sans-serif;");
    auto *centralWidget = new QWidget (this);
    centralWidget->setWindowTitle (tr ("My Page"));

    QVBoxLayout *userProfileLayout = new QVBoxLayout (centralWidget);
    userProfileLayout->setAlignment (Qt::AlignCenter | Qt::AlignTop);
    userProfileLayout->setSpacing (20);
    userProfileLayout->setContentsMargins (5, 20, 5, 20);

    QHBoxLayout *avatarLayout = new QHBoxLayout (centralWidget);
    QLabel *avatarTextLabel = new QLabel (tr ("User Avatar: "), centralWidget);
    avatarTextLabel->setStyleSheet (
        QString ("font-size: %1px; font-weight: normal; color: #333;")
            .arg (Constants::Settings::SUBTITLE_FONT_SIZE));

    avatarTextLabel->setFont (QFont (Constants::Settings::DEFAULT_FONT_FAMILY,
                                     Constants::Settings::SUBTITLE_FONT_SIZE,
                                     QFont::Normal));

    avatarLayout->addWidget (avatarTextLabel);

    avatarLabel = new QLabel (centralWidget);

    avatarLabel->setFixedSize (100, 100);
    avatarLabel->setPixmap (
        QPixmap (Constants::Resources::DEFAULT_USER_AVATAR));
    avatarLabel->setScaledContents (true);

    avatarLayout->addWidget (avatarLabel);

    ElaPushButton *changeAvatarButton =
        new ElaPushButton (tr ("Change Avatar"), centralWidget);
    changeAvatarButton->setFixedSize (150, 40);
    changeAvatarButton->setStyleSheet (
        QString ("background-color: #4CAF50; color: white; font-size: %1px; "
                 "border-radius: 5px; padding: 10px;")
            .arg (Constants::Settings::DEFAULT_FONT_SIZE));
    changeAvatarButton->setFont (
        QFont (Constants::Settings::DEFAULT_FONT_FAMILY,
               Constants::Settings::DEFAULT_FONT_SIZE, QFont::Normal));

    avatarLayout->addWidget (changeAvatarButton);

    userProfileLayout->addLayout (avatarLayout);

    auto *splitLine1 = new QFrame (centralWidget);
    splitLine1->setFrameShape (QFrame::HLine);
    splitLine1->setFrameShadow (QFrame::Sunken);
    splitLine1->setStyleSheet ("background-color: #ccc; margin: 10px 0;");
    userProfileLayout->addWidget (splitLine1);

    QHBoxLayout *usernameLayout = new QHBoxLayout (centralWidget);

    QLabel *usernameTextLabel = new QLabel ();

    usernameTextLabel->setText (tr ("Username: "));

    usernameTextLabel->setStyleSheet (
        QString ("font-size: %1px; font-weight: normal; color: #333;")
            .arg (Constants::Settings::SUBTITLE_FONT_SIZE));
    usernameTextLabel->setFont (QFont (Constants::Settings::DEFAULT_FONT_FAMILY,
                                       Constants::Settings::SUBTITLE_FONT_SIZE,
                                       QFont::Normal));

    usernameLayout->addWidget (usernameTextLabel);

    usernameLineEdit = new ElaLineEdit (centralWidget);

    usernameLineEdit->setPlaceholderText (tr ("Enter your username"));

    usernameLineEdit->setStyleSheet (
        QString ("font-size: %1px; color: #333; padding: 5px; "
                 "border: 1px solid #ccc; border-radius: 5px;")
            .arg (Constants::Settings::DEFAULT_FONT_SIZE));
    usernameLineEdit->setFont (QFont (Constants::Settings::DEFAULT_FONT_FAMILY,
                                      Constants::Settings::SUBTITLE_FONT_SIZE,
                                      QFont::Normal));

    usernameLineEdit->setText (AccountManager::getInstance ().getUsername ());

    usernameLineEdit->setFixedWidth (400);

    usernameLayout->addWidget (usernameLineEdit);

    ElaPushButton *changeUsernameButton =
        new ElaPushButton (tr ("Change Username"), centralWidget);

    usernameLayout->addWidget (changeUsernameButton);

    userProfileLayout->addLayout (usernameLayout);

    QHBoxLayout *passwordLayout = new QHBoxLayout (centralWidget);
    QLabel *passwordTextLabel =
        new QLabel (tr ("Password: ******"), centralWidget);
    passwordTextLabel->setStyleSheet (
        QString ("font-size: %1px; font-weight: normal; color: #333;")
            .arg (Constants::Settings::SUBTITLE_FONT_SIZE));
    passwordTextLabel->setFont (QFont (Constants::Settings::DEFAULT_FONT_FAMILY,
                                       Constants::Settings::SUBTITLE_FONT_SIZE,
                                       QFont::Normal));
    passwordLayout->addWidget (passwordTextLabel);

    ElaPushButton *changePasswordButton =
        new ElaPushButton (tr ("Change Password"), centralWidget);

    passwordLayout->addWidget (changePasswordButton);
    userProfileLayout->addLayout (passwordLayout);

    QHBoxLayout *emailLayout = new QHBoxLayout (centralWidget);

    QLabel *emailTextLabel = new QLabel (tr ("Email: "), centralWidget);

    emailTextLabel->setStyleSheet (
        "font-size: 20px; font-weight: normal; color: #333;");
    emailTextLabel->setFont (QFont (Constants::Settings::DEFAULT_FONT_FAMILY,
                                    Constants::Settings::TITLE_FONT_SIZE));

    emailLayout->addWidget (emailTextLabel);

    emailLineEdit = new ElaLineEdit (centralWidget);

    emailLineEdit->setPlaceholderText (tr ("Enter your email"));

    emailLineEdit->setStyleSheet (
        "font-size: 15px; color: #333; padding: 5px; "
        "border: 1px solid #ccc; border-radius: 5px;");
    emailLineEdit->setFont (QFont (Constants::Settings::DEFAULT_FONT_FAMILY,
                                   Constants::Settings::TITLE_FONT_SIZE));

    emailLineEdit->setText (AccountManager::getInstance ().getEmail ());

    emailLineEdit->setFixedWidth (400);

    emailLayout->addWidget (emailLineEdit);

    ElaPushButton *changeEmailButton =
        new ElaPushButton (tr ("Change Email"), centralWidget);
    emailLayout->addWidget (changeEmailButton);
    userProfileLayout->addLayout (emailLayout);

    auto *splitLine2 = new QFrame (centralWidget);
    splitLine2->setFrameShape (QFrame::HLine);
    splitLine2->setFrameShadow (QFrame::Sunken);
    splitLine2->setStyleSheet ("background-color: #ccc; margin: 10px 0;");

    userProfileLayout->addWidget (splitLine2);

    QHBoxLayout *logoutLayout = new QHBoxLayout (centralWidget);
    QPushButton *logoutButton = new QPushButton (tr ("Logout"), centralWidget);
    logoutButton->setStyleSheet ("QPushButton { "
                                 "color: #F44336; "
                                 "font-size: 16px; "
                                 "border: 2px solid #F44336; "
                                 "border-radius: 5px; "
                                 "padding: 10px; "
                                 "background-color: transparent; } "
                                 "QPushButton:hover { "
                                 "border-color: #0078d7; "
                                 "color: #0078d7; }");
    logoutButton->setFont (QFont (Constants::Settings::DEFAULT_FONT_FAMILY,
                                  Constants::Settings::DEFAULT_FONT_SIZE));
    logoutLayout->addWidget (logoutButton);
    userProfileLayout->addLayout (logoutLayout);

    connect (changeAvatarButton, &ElaPushButton::clicked, this,
             [this] ()
             {
                 auto result = AccountManager::getInstance ().changeAvatar ();
                 if (result == ChangeResult::Success)
                 {
                     showDialog (tr ("Success"),
                                 tr ("Avatar changed successfully."));
                 }
                 else
                 {
                     auto it = ChangeResultMessage.find (result);
                     QString errorMsg =
                         it != ChangeResultMessage.end ()
                             ? QString::fromStdString (it->second)
                             : tr ("Unknown error");
                     showDialog (tr ("Change Avatar Error"), errorMsg);
                 }
             });

    connect (
        changeUsernameButton, &ElaPushButton::clicked, this,
        [this] ()
        {
            if (usernameLineEdit->text ().isEmpty ())
            {
                showDialog (tr ("Error"), tr ("Username cannot be empty."));
                return;
            }

            auto result = AccountManager::getInstance ().changeUsername (
                usernameLineEdit->text ());

            if (result == ChangeResult::Success)
            {
                showDialog (tr ("Success"),
                            tr ("Username changed successfully."));
                emit usernameChanged (usernameLineEdit->text ());
            }
            else
            {
                auto it = ChangeResultMessage.find (result);
                QString errorMsg = it != ChangeResultMessage.end ()
                                       ? QString::fromStdString (it->second)
                                       : tr ("Unknown error");
                showDialog (tr ("Change Username Error"), errorMsg);
            }
        });

    connect (changePasswordButton, &ElaPushButton::clicked, this,
             [this] ()
             {
                 showDialog (
                     tr ("Change Password"),
                     [this] ()
                     {
                         if (oldPasswordLineEdit->text ().isEmpty () ||
                             newPasswordLineEdit->text ().isEmpty ())
                         {
                             showDialog (tr ("Error"),
                                         tr ("Fields cannot be empty."));
                             return;
                         }

                         auto oldPasswordHash = AccountManager::hashPassword (
                             oldPasswordLineEdit->text ());

                         auto newPasswordHash = AccountManager::hashPassword (
                             newPasswordLineEdit->text ());

                         auto result =
                             AccountManager::getInstance ().changePassword (
                                 oldPasswordHash, newPasswordHash);

                         if (result == ChangeResult::Success)
                         {
                             showDialog (tr ("Success"),
                                         tr ("Password changed successfully."));
                         }
                         else
                         {
                             auto it = ChangeResultMessage.find (result);
                             QString errorMsg =
                                 it != ChangeResultMessage.end ()
                                     ? QString::fromStdString (it->second)
                                     : tr ("Unknown error");
                             showDialog (tr ("Change Password Error"),
                                         errorMsg);
                         }
                     });
             });

    connect (
        changeEmailButton, &ElaPushButton::clicked, this,
        [this] ()
        {
            if (emailLineEdit->text ().isEmpty ())
            {
                showDialog (tr ("Error"), tr ("Email cannot be empty."));
                return;
            }

            if (!emailLineEdit->text ().contains ("@"))
            {
                showDialog (tr ("Error"), tr ("Invalid email format."));
                return;
            }

            auto result = AccountManager::getInstance ().changeEmail (
                emailLineEdit->text ());
            if (result == ChangeResult::Success)
            {
                showDialog (tr ("Success"), tr ("Email changed successfully."));
                emit emailChanged (emailLineEdit->text ());
            }
            else
            {
                auto it = ChangeResultMessage.find (result);
                QString errorMsg = it != ChangeResultMessage.end ()
                                       ? QString::fromStdString (it->second)
                                       : tr ("Unknown error");
                showDialog (tr ("Change Email Error"), errorMsg);
            }
        });

    connect (
        logoutButton, &QPushButton::clicked, this,
        [this] ()
        {
            showDialog (
                tr ("Confirm Logout"), tr ("Are you sure you want to logout?"),
                [this] ()
                {
                    AccountManager::getInstance ().logout ();
                    emit AccountManager::getInstance ().logoutSuccessful ();
                });
        });

    addCentralWidget (centralWidget, true, true, 0);
}
