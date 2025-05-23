#include "LoginPage.h"
#include "ElaContentDialog.h"
#include <qboxlayout.h>
#include <qlabel.h>
#include <qlogging.h>
#include <qnamespace.h>
LoginPage::LoginPage (QWidget *parent) : TempPage (parent)
{
    setWindowTitle ("Login&Register");
    setWindowFlag (Qt::WindowStaysOnTopHint, true);
    setWindowModality (Qt::ApplicationModal);

    auto *centralWidget = new QWidget (this);
    centralWidget->setWindowTitle ("Login");
    resize (858, 600);
    setMinimumSize (644, 450);
    setMaximumSize (644, 450);

    QVBoxLayout *loginPageLayout = new QVBoxLayout (centralWidget);
    loginPageLayout->setAlignment (Qt::AlignHCenter | Qt::AlignTop);
    loginPageLayout->setSpacing (20);
    loginPageLayout->setContentsMargins (15, 40, 15, 40);

    QLabel *titleLabel = new QLabel ("Welcome to TranCE_Wyatt", centralWidget);
    titleLabel->setAlignment (Qt::AlignHCenter);
    titleLabel->setStyleSheet (
        "font-size: 24px; font-weight: bold; color: #333;");
    titleLabel->setFont (QFont ("Noto Sans", 24, QFont::Bold));

    loginPageLayout->addWidget (titleLabel);
    ElaLineEdit *usernameLineEdit = new ElaLineEdit (centralWidget);
    usernameLineEdit->setPlaceholderText ("Username");
    usernameLineEdit->setMinimumHeight (50);
    usernameLineEdit->setMaximumHeight (50);
    usernameLineEdit->setMinimumWidth (450);
    usernameLineEdit->setMaximumWidth (450);
    usernameLineEdit->setBorderRadius (20);

    loginPageLayout->addWidget (usernameLineEdit);

    ElaLineEdit *passwordLineEdit = new ElaLineEdit (centralWidget);
    passwordLineEdit->setPlaceholderText ("Password");
    passwordLineEdit->setMinimumHeight (50);
    passwordLineEdit->setMaximumHeight (50);
    passwordLineEdit->setMinimumWidth (450);
    passwordLineEdit->setMaximumWidth (450);
    passwordLineEdit->setBorderRadius (20);
    passwordLineEdit->setEchoMode (ElaLineEdit::Password);

    loginPageLayout->addWidget (passwordLineEdit);

    QHBoxLayout *loginButtonLayout = new QHBoxLayout (centralWidget);
    loginButtonLayout->setAlignment (Qt::AlignHCenter);
    loginButtonLayout->setSpacing (20);

    ElaPushButton *loginButton = new ElaPushButton (centralWidget);
    loginButton->setText ("Login");
    loginButton->setMinimumHeight (50);
    loginButton->setMaximumHeight (50);
    loginButton->setMinimumWidth (200);
    loginButton->setMaximumWidth (200);
    loginButton->setBorderRadius (20);
    loginButtonLayout->addWidget (loginButton);

    ElaPushButton *registerButton = new ElaPushButton (centralWidget);
    registerButton->setText ("Register");
    registerButton->setMinimumHeight (50);
    registerButton->setMaximumHeight (50);
    registerButton->setMinimumWidth (200);
    registerButton->setMaximumWidth (200);
    registerButton->setBorderRadius (20);

    loginButtonLayout->addWidget (registerButton);

    loginPageLayout->addLayout (loginButtonLayout);
    addCentralWidget (centralWidget, true, true, 0);

    connect (loginButton, &ElaPushButton::clicked,
             [=, this] ()
             {
                 QString username = usernameLineEdit->text ();
                 QString password = passwordLineEdit->text ();

                 if (username.isEmpty () || password.isEmpty ())
                 {
                     showDialog ("Login Error",
                                 "Username or password cannot be empty.\n\n"
                                 "Please enter your username/password.");
                     return;
                 }

                 try
                 {
                     AccountManager::getInstance ().login (username, password);
                     // Handle successful login
                 }
                 catch (const std::runtime_error &e)
                 {
                     // Handle login error
                     qCritical () << "Login error:" << e.what ();
                 }
             });

    connect (registerButton, &ElaPushButton::clicked,
             [=, this] ()
             {
                 auto *registerPage = new RegisterPage (this);

                 registerPage->show ();
                 registerPage->setAttribute (Qt::WA_DeleteOnClose);
             });
}

void LoginPage::paintEvent (QPaintEvent *event)
{
    Q_UNUSED (event);
    QPainter painter (this);
    painter.setRenderHint (QPainter::Antialiasing, true);
    painter.setRenderHint (QPainter::TextAntialiasing, true);
    painter.setRenderHint (QPainter::SmoothPixmapTransform, true);

    painter.setPen (Qt::black);
}