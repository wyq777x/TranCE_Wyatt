#include "LoginPage.h"
#include "Controller/AccountManager.h"
#include "ElaLineEdit.h"
#include "ElaPushButton.h"
#include "Utility/Constants.h"
#include "Utility/Result.h"

LoginPage::LoginPage (QWidget *parent) : TempPage (parent)
{
    setWindowTitle (tr ("Login&Register"));
    setWindowFlag (Qt::WindowStaysOnTopHint, true);
    setWindowModality (Qt::ApplicationModal);

    auto *centralWidget = new QWidget (this);
    centralWidget->setWindowTitle (tr ("Login"));
    resize (858, 600);
    setMinimumSize (644, 450);
    setMaximumSize (644, 450);

    QVBoxLayout *loginPageLayout = new QVBoxLayout (centralWidget);
    loginPageLayout->setAlignment (Qt::AlignHCenter | Qt::AlignTop);
    loginPageLayout->setSpacing (20);
    loginPageLayout->setContentsMargins (15, 40, 15, 40);

    QLabel *titleLabel =
        new QLabel (tr ("Welcome to TranCE_Wyatt"), centralWidget);
    titleLabel->setAlignment (Qt::AlignHCenter);
    titleLabel->setStyleSheet (
        QString ("font-size: %1px; font-weight: bold; color: #333;")
            .arg (Constants::Settings::TITLE_FONT_SIZE));
    titleLabel->setFont (QFont (Constants::Settings::DEFAULT_FONT_FAMILY,
                                Constants::Settings::TITLE_FONT_SIZE,
                                QFont::Bold));

    loginPageLayout->addWidget (titleLabel);
    ElaLineEdit *usernameLineEdit = new ElaLineEdit (centralWidget);
    usernameLineEdit->setPlaceholderText (tr ("Username"));
    usernameLineEdit->setMinimumHeight (Constants::UI::DEFAULT_INPUT_HEIGHT);
    usernameLineEdit->setMaximumHeight (Constants::UI::DEFAULT_INPUT_HEIGHT);
    usernameLineEdit->setMinimumWidth (Constants::UI::DEFAULT_INPUT_WIDTH);
    usernameLineEdit->setMaximumWidth (Constants::UI::DEFAULT_INPUT_WIDTH);
    usernameLineEdit->setBorderRadius (Constants::UI::DEFAULT_BORDER_RADIUS);

    loginPageLayout->addWidget (usernameLineEdit);

    ElaLineEdit *passwordLineEdit = new ElaLineEdit (centralWidget);
    passwordLineEdit->setPlaceholderText (tr ("Password"));
    passwordLineEdit->setMinimumHeight (Constants::UI::DEFAULT_INPUT_HEIGHT);
    passwordLineEdit->setMaximumHeight (Constants::UI::DEFAULT_INPUT_HEIGHT);
    passwordLineEdit->setMinimumWidth (Constants::UI::DEFAULT_INPUT_WIDTH);
    passwordLineEdit->setMaximumWidth (Constants::UI::DEFAULT_INPUT_WIDTH);
    passwordLineEdit->setBorderRadius (Constants::UI::DEFAULT_BORDER_RADIUS);
    passwordLineEdit->setEchoMode (ElaLineEdit::Password);

    loginPageLayout->addWidget (passwordLineEdit);

    QHBoxLayout *loginButtonLayout = new QHBoxLayout (centralWidget);
    loginButtonLayout->setAlignment (Qt::AlignHCenter);
    loginButtonLayout->setSpacing (20);

    ElaPushButton *loginButton = new ElaPushButton (centralWidget);
    loginButton->setText (tr ("Login"));
    loginButton->setMinimumHeight (Constants::UI::DEFAULT_BUTTON_HEIGHT);
    loginButton->setMaximumHeight (Constants::UI::DEFAULT_BUTTON_HEIGHT);
    loginButton->setMinimumWidth (200);
    loginButton->setMaximumWidth (200);
    loginButton->setBorderRadius (Constants::UI::DEFAULT_BORDER_RADIUS);
    loginButtonLayout->addWidget (loginButton);

    ElaPushButton *registerButton = new ElaPushButton (centralWidget);
    registerButton->setText (tr ("Register"));
    registerButton->setMinimumHeight (Constants::UI::DEFAULT_BUTTON_HEIGHT);
    registerButton->setMaximumHeight (Constants::UI::DEFAULT_BUTTON_HEIGHT);
    registerButton->setMinimumWidth (200);
    registerButton->setMaximumWidth (200);
    registerButton->setBorderRadius (Constants::UI::DEFAULT_BORDER_RADIUS);

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
                     showDialog (tr ("Login Error"),
                                 tr ("Username or password cannot be empty.\n\n"
                                     "Please enter your username/password."));
                     return;
                 }

                 try
                 {
                     auto result = AccountManager::getInstance ().login (
                         username, password);

                     if (result != UserAuthResult::Success)
                     {
                         auto it = UserAuthResultMessage.find (result);
                         QString errorMsg =
                             it != UserAuthResultMessage.end ()
                                 ? QString::fromStdString (it->second)
                                 : tr ("Unknown error");
                         showDialog (tr ("Login Error"), errorMsg);

                         return;
                     }
                     else
                     {
                         showDialog (tr ("Login Success"),
                                     tr ("Welcome back, %1!").arg (username));
                     }
                 }
                 catch (const std::exception &e)
                 {
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