#include "View/Components/RegisterPage.h"
#include "ElaLineEdit.h"
#include <qdialogbuttonbox.h>
#include <qlabel.h>

RegisterPage::RegisterPage (QWidget *parent) : TempPage (parent)
{
    setWindowTitle ("Register");
    setWindowFlag (Qt::Window, true);
    setWindowModality (Qt::ApplicationModal);

    auto *centralWidget = new QWidget (nullptr);
    centralWidget->setWindowTitle ("Register");
    resize (858, 600);
    setMinimumSize (644, 450);
    setMaximumSize (644, 450);

    QVBoxLayout *registerPageLayout = new QVBoxLayout (centralWidget);
    registerPageLayout->setAlignment (Qt::AlignHCenter | Qt::AlignTop);
    registerPageLayout->setSpacing (20);
    registerPageLayout->setContentsMargins (15, 40, 15, 40);

    QLabel *titleLabel = new QLabel ("RegisterUser", centralWidget);
    titleLabel->setAlignment (Qt::AlignHCenter);
    titleLabel->setStyleSheet (
        "font-size: 24px; font-weight: bold; color: #333;");
    titleLabel->setFont (QFont ("Noto Sans", 24, QFont::Bold));

    registerPageLayout->addWidget (titleLabel);

    ElaLineEdit *usernameLineEdit = new ElaLineEdit (centralWidget);
    usernameLineEdit->setPlaceholderText ("Username");
    usernameLineEdit->setMinimumHeight (50);
    usernameLineEdit->setMaximumHeight (50);
    usernameLineEdit->setMinimumWidth (450);
    usernameLineEdit->setMaximumWidth (450);
    usernameLineEdit->setBorderRadius (20);

    registerPageLayout->addWidget (usernameLineEdit);

    ElaLineEdit *passwordLineEdit = new ElaLineEdit (centralWidget);

    passwordLineEdit->setPlaceholderText ("Password");
    passwordLineEdit->setMinimumHeight (50);
    passwordLineEdit->setMaximumHeight (50);
    passwordLineEdit->setMinimumWidth (450);
    passwordLineEdit->setMaximumWidth (450);
    passwordLineEdit->setBorderRadius (20);
    passwordLineEdit->setEchoMode (ElaLineEdit::Password);

    registerPageLayout->addWidget (passwordLineEdit);

    ElaLineEdit *confirmPasswordLineEdit = new ElaLineEdit (centralWidget);
    confirmPasswordLineEdit->setPlaceholderText ("Confirm Password");
    confirmPasswordLineEdit->setMinimumHeight (50);
    confirmPasswordLineEdit->setMaximumHeight (50);
    confirmPasswordLineEdit->setMinimumWidth (450);
    confirmPasswordLineEdit->setMaximumWidth (450);
    confirmPasswordLineEdit->setBorderRadius (20);
    confirmPasswordLineEdit->setEchoMode (ElaLineEdit::Password);

    registerPageLayout->addWidget (confirmPasswordLineEdit);

    QHBoxLayout *registerButtonLayout = new QHBoxLayout (centralWidget);
    registerButtonLayout->setAlignment (Qt::AlignHCenter);
    registerButtonLayout->setSpacing (20);

    ElaPushButton *registerButton = new ElaPushButton (centralWidget);
    registerButton->setText ("Register");
    registerButton->setMinimumHeight (50);
    registerButton->setMaximumHeight (50);
    registerButton->setMinimumWidth (200);
    registerButton->setMaximumWidth (200);
    registerButton->setBorderRadius (20);

    registerButtonLayout->addWidget (registerButton);

    ElaPushButton *cancelButton = new ElaPushButton (centralWidget);
    cancelButton->setText ("Cancel");
    cancelButton->setMinimumHeight (50);
    cancelButton->setMaximumHeight (50);
    cancelButton->setMinimumWidth (200);
    cancelButton->setMaximumWidth (200);
    cancelButton->setBorderRadius (20);

    registerButtonLayout->addWidget (cancelButton);

    registerPageLayout->addLayout (registerButtonLayout);

    addCentralWidget (centralWidget, true, true, 0);

    connect (
        registerButton, &ElaPushButton::clicked,
        [=, this] ()
        {
            QString username = usernameLineEdit->text ();
            QString password = passwordLineEdit->text ();
            QString confirmPassword = confirmPasswordLineEdit->text ();

            if (username.isEmpty () || password.isEmpty ())
            {
                showDialog ("Register Error",
                            "Username or password cannot be empty.\n\n"
                            "Please enter your username/password.");
                return;
            }

            if (password != confirmPassword)
            {
                showDialog ("Register Error", "Passwords do not match.\n\n"
                                              "Please confirm your password.");
                return;
            }

            auto result = AccountManager::getInstance ().registerUser (
                username, password);

            if (result != RegisterUserResult::Success)
            {
                auto it = RegisterUserResultMessage.find (result);
                QString errorMsg = it != RegisterUserResultMessage.end ()
                                       ? QString::fromStdString (it->second)
                                       : "Unknown error";
                showDialog ("Register Error", errorMsg);
                return;
            }
            else
            {
                // invoke UserModel to create user profile and settings
                UserModel::getInstance ().createUserData (
                    "profile_" +
                        AccountManager::getInstance ().getUserUuid (username) +
                        ".json",
                    username);

                showDialog (
                    "Register Success",
                    "User registered successfully.\n\n"
                    "You can now log in with your username and password.");
            }
        });

    connect (cancelButton, &ElaPushButton::clicked, [=, this] () { close (); });
}
void RegisterPage::paintEvent (QPaintEvent *event)
{
    Q_UNUSED (event);
    QPainter painter (this);
    painter.setRenderHint (QPainter::Antialiasing, true);
    painter.setRenderHint (QPainter::TextAntialiasing, true);
    painter.setRenderHint (QPainter::SmoothPixmapTransform, true);

    painter.setPen (Qt::black);
}
