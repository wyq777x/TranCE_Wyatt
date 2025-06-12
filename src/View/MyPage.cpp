#include "MyPage.h"
#include "AccountManager.h"
#include "DbManager.h"
#include "ElaLineEdit.h"
#include "ElaPushButton.h"
#include "Utility/Result.h"
#include <qnamespace.h>

MyPage::MyPage (QWidget *parent) : TempPage (parent)
{
    setWindowTitle ("My Page");
    setMinimumSize (800, 600);
    setMaximumSize (800, 600);
    setStyleSheet (
        "background-color: #f0f0f0; font-family: 'Noto Sans', sans-serif;");
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
    avatarLabel->setPixmap (QPixmap (":image/DefaultUser.png"));

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

    auto *splitLine1 = new QFrame (centralWidget);
    splitLine1->setFrameShape (QFrame::HLine);
    splitLine1->setFrameShadow (QFrame::Sunken);
    splitLine1->setStyleSheet ("background-color: #ccc; margin: 10px 0;");
    userProfileLayout->addWidget (splitLine1);

    QHBoxLayout *usernameLayout = new QHBoxLayout (centralWidget);

    QLabel *usernameTextLabel = new QLabel ();

    usernameTextLabel->setText (QString ("Username: "));

    usernameTextLabel->setStyleSheet (
        "font-size: 20px; font-weight: normal; color: #333;");
    usernameTextLabel->setFont (QFont ("Noto Sans", 24));

    usernameLayout->addWidget (usernameTextLabel);

    usernameLineEdit = new ElaLineEdit (centralWidget);

    usernameLineEdit->setPlaceholderText ("Enter your username");

    usernameLineEdit->setStyleSheet (
        "font-size: 15px; color: #333; padding: 5px; "
        "border: 1px solid #ccc; border-radius: 5px;");
    usernameLineEdit->setFont (QFont ("Noto Sans", 24));

    usernameLineEdit->setText (AccountManager::getInstance ().getUsername ());

    usernameLineEdit->setFixedWidth (400);

    usernameLayout->addWidget (usernameLineEdit);

    ElaPushButton *changeUsernameButton =
        new ElaPushButton ("Change Username", centralWidget);

    usernameLayout->addWidget (changeUsernameButton);

    userProfileLayout->addLayout (usernameLayout);

    QHBoxLayout *passwordLayout = new QHBoxLayout (centralWidget);
    QLabel *passwordTextLabel = new QLabel ("Password: ******", centralWidget);
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

    emailLineEdit = new ElaLineEdit (centralWidget);

    emailLineEdit->setPlaceholderText ("Enter your email");

    emailLineEdit->setStyleSheet (
        "font-size: 15px; color: #333; padding: 5px; "
        "border: 1px solid #ccc; border-radius: 5px;");
    emailLineEdit->setFont (QFont ("Noto Sans", 24));

    emailLineEdit->setText (AccountManager::getInstance ().getEmail ());

    emailLineEdit->setFixedWidth (400);

    emailLayout->addWidget (emailLineEdit);

    ElaPushButton *changeEmailButton =
        new ElaPushButton ("Change Email", centralWidget);
    emailLayout->addWidget (changeEmailButton);
    userProfileLayout->addLayout (emailLayout);

    auto *splitLine2 = new QFrame (centralWidget);
    splitLine2->setFrameShape (QFrame::HLine);
    splitLine2->setFrameShadow (QFrame::Sunken);
    splitLine2->setStyleSheet ("background-color: #ccc; margin: 10px 0;");

    userProfileLayout->addWidget (splitLine2);

    QHBoxLayout *logoutLayout = new QHBoxLayout (centralWidget);
    ElaPushButton *logoutButton = new ElaPushButton ("Logout", centralWidget);
    logoutButton->setStyleSheet (
        "background-color: #F44336; color: white; font-size: 16px; "
        "border-radius: 5px; padding: 10px;");
    logoutButton->setFont (QFont ("Noto Sans", 16));
    logoutLayout->addWidget (logoutButton);
    userProfileLayout->addLayout (logoutLayout);

    connect (changeAvatarButton, &ElaPushButton::clicked, this,
                 [] () { 


                 });

    connect (changeUsernameButton, &ElaPushButton::clicked, this,
             [this] ()
             {
                 if (usernameLineEdit->text ().isEmpty ())
                 {
                     showDialog ("Error", "Username cannot be empty.");
                     return;
                 }

                 auto result = AccountManager::getInstance ().changeUsername (
                     usernameLineEdit->text ());

                 if (result == ChangeResult::Success)
                 {
                     showDialog ("Success", "Username changed successfully.");
                 }
                 else
                 {
                     auto it = ChangeResultMessage.find (result);
                     QString errorMsg =
                         it != ChangeResultMessage.end ()
                             ? QString::fromStdString (it->second)
                             : "Unknown error";
                     showDialog ("Change Username Error", errorMsg);
                 }
             });

    connect (
        changePasswordButton, &ElaPushButton::clicked, this,
        [this] ()
        {
            showDialog (
                "Change Password",
                [this] ()
                {
                    if (oldPasswordLineEdit->text ().isEmpty () ||
                        newPasswordLineEdit->text ().isEmpty ())
                    {
                        showDialog ("Error", "Fields cannot be empty.");
                        return;
                    }

                    auto oldPasswordHash = AccountManager::hashPassword (
                        oldPasswordLineEdit->text ());

                    auto newPasswordHash = AccountManager::hashPassword (
                        newPasswordLineEdit->text ());

                    if (oldPasswordHash !=
                        AccountManager::getInstance ().getHashedPassword ())
                    {
                        showDialog ("Error", "Old password is incorrect.");
                        return;
                    }

                    auto result =
                        AccountManager::getInstance ().changePassword (
                            newPasswordHash);

                    if (result == ChangeResult::Success)
                    {
                        showDialog ("Success",
                                    "Password changed successfully.");
                    }
                    else
                    {
                        auto it = ChangeResultMessage.find (result);
                        QString errorMsg =
                            it != ChangeResultMessage.end ()
                                ? QString::fromStdString (it->second)
                                : "Unknown error";
                        showDialog ("Change Password Error", errorMsg);
                    }
                });
        });

    connect (changeEmailButton, &ElaPushButton::clicked, this,
             [this] ()
             {
                 if (emailLineEdit->text ().isEmpty ())
                 {
                     showDialog ("Error", "Email cannot be empty.");
                     return;
                 }

                 if (!emailLineEdit->text ().contains ("@"))
                 {
                     showDialog ("Error", "Invalid email format.");
                     return;
                 }

                 auto result = AccountManager::getInstance ().changeEmail (
                     emailLineEdit->text ());
                 if (result == ChangeResult::Success)
                 {
                     showDialog ("Success", "Email changed successfully.");
                 }
                 else
                 {
                     auto it = ChangeResultMessage.find (result);
                     QString errorMsg =
                         it != ChangeResultMessage.end ()
                             ? QString::fromStdString (it->second)
                             : "Unknown error";
                     showDialog ("Change Email Error", errorMsg);
                 }
             });

    connect (
        logoutButton, &ElaPushButton::clicked, this,
        [this] ()
        {
            showDialog (
                "Confirm Logout", "Are you sure you want to logout?",
                [this] ()
                {
                    AccountManager::getInstance ().logout ();
                    emit AccountManager::getInstance ().logoutSuccessful ();
                });
        });

    addCentralWidget (centralWidget, true, true, 0);
}
