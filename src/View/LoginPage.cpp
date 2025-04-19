#include "LoginPage.h"
#include "ElaFlowLayout.h"
#include "ElaLineEdit.h"
#include "ElaPushButton.h"
#include <qboxlayout.h>
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

    QLabel *titleLabel =
        new QLabel ("           Welcome to TranCE_Wyatt", centralWidget);
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

    loginButtonLayout->addWidget (loginButton);

    ElaPushButton *registerButton = new ElaPushButton (centralWidget);
    registerButton->setText ("Register");

    loginButtonLayout->addWidget (registerButton);

    loginPageLayout->addLayout (loginButtonLayout);
    addCentralWidget (centralWidget, true, true, 0);
}

void LoginPage::LoginPage::paintEvent (QPaintEvent *event)
{
    Q_UNUSED (event);
    QPainter painter (this);
    painter.setRenderHint (QPainter::Antialiasing, true);
    painter.setRenderHint (QPainter::TextAntialiasing, true);
    painter.setRenderHint (QPainter::SmoothPixmapTransform, true);

    painter.setPen (Qt::black);
}