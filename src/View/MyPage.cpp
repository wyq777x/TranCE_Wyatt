#include "MyPage.h"
#include <qnamespace.h>

MyPage::MyPage (QWidget *parent) : TempPage (parent)
{
    setWindowTitle ("My Page");
    setMinimumSize (800, 600);
    setMaximumSize (800, 600);
    setStyleSheet ("QWidget { background-color: "
                   "white; border-radius: 10px;}");

    auto *centralWidget = new QWidget (this);
    centralWidget->setWindowTitle ("My Page");

    QVBoxLayout *userProfileLayout = new QVBoxLayout (centralWidget);
    userProfileLayout->setAlignment (Qt::AlignCenter | Qt::AlignTop);
    userProfileLayout->setSpacing (20);
    userProfileLayout->setContentsMargins (5, 20, 5, 20);

    QHBoxLayout *avatarLayout = new QHBoxLayout (centralWidget);
    QLabel *avatarTextLabel = new QLabel ("User Avatar: ", centralWidget);
    avatarTextLabel->setStyleSheet (
        "font-size: 20px; font-weight: normal; color: #333;");

    avatarTextLabel->setFont (QFont ("Noto Sans", 24));

    avatarLayout->addWidget (avatarTextLabel);

    QLabel *avatarLabel = new QLabel (centralWidget);
    avatarLabel->setPixmap (QPixmap (":/res/image/DefaultUser.png"));

    avatarLayout->addWidget (avatarLabel);

    ElaPushButton *changeAvatarButton =
        new ElaPushButton ("Change Avatar", centralWidget);
    changeAvatarButton->setFixedSize (150, 40);
    changeAvatarButton->setStyleSheet (
        "background-color: #4CAF50; color: white; font-size: 16px; "
        "border-radius: 5px; padding: 10px;");
    changeAvatarButton->setFont (QFont ("Noto Sans", 16));

    avatarLayout->addWidget (changeAvatarButton);

    userProfileLayout->addLayout (avatarLayout);

    QHBoxLayout *usernameLayout = new QHBoxLayout (centralWidget);
    QLabel *usernameTextLabel = new QLabel (centralWidget);
    usernameTextLabel->setText (
        QString ("Username: %1")
            .arg (AccountManager::getInstance ().getUsername ()));

    usernameTextLabel->setStyleSheet (
        "font-size: 20px; font-weight: normal; color: #333;");
    usernameTextLabel->setFont (QFont ("Noto Sans", 24));

    usernameLayout->addWidget (usernameTextLabel);

    ElaPushButton *changeUsernameButton =
        new ElaPushButton ("Change Username", centralWidget);

    usernameLayout->addWidget (changeUsernameButton);

    userProfileLayout->addLayout (usernameLayout);

    QHBoxLayout *passwordLayout = new QHBoxLayout (centralWidget);
    QLabel *passwordTextLabel = new QLabel ("Password: ", centralWidget);
    passwordTextLabel->setStyleSheet (
        "font-size: 20px; font-weight: normal; color: #333;");
    passwordTextLabel->setFont (QFont ("Noto Sans", 24));
    passwordLayout->addWidget (passwordTextLabel);

    ElaPushButton *changePasswordButton =
        new ElaPushButton ("Change Password", centralWidget);

    passwordLayout->addWidget (changePasswordButton);
    userProfileLayout->addLayout (passwordLayout);

    QHBoxLayout *emailLayout = new QHBoxLayout (centralWidget);
    QLabel *emailTextLabel = new QLabel ("Email: ", centralWidget);
    emailTextLabel->setStyleSheet (
        "font-size: 20px; font-weight: normal; color: #333;");
    emailTextLabel->setFont (QFont ("Noto Sans", 24));
    emailLayout->addWidget (emailTextLabel);
    ElaPushButton *changeEmailButton =
        new ElaPushButton ("Change Email", centralWidget);
    emailLayout->addWidget (changeEmailButton);
    userProfileLayout->addLayout (emailLayout);

    QHBoxLayout *logoutLayout = new QHBoxLayout (centralWidget);
    ElaPushButton *logoutButton = new ElaPushButton ("Logout", centralWidget);
    logoutButton->setStyleSheet (
        "background-color: #F44336; color: white; font-size: 16px; "
        "border-radius: 5px; padding: 10px;");
    logoutButton->setFont (QFont ("Noto Sans", 16));
    logoutLayout->addWidget (logoutButton);
    userProfileLayout->addLayout (logoutLayout);

    connect (logoutButton, &ElaPushButton::clicked, this,
             [] () { AccountManager::getInstance ().logout (); });

    addCentralWidget (centralWidget, true, true, 0);
}
