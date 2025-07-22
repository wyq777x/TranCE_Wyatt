#include "RegisterPage.h"
#include "Controller/AccountManager.h"
#include "ElaLineEdit.h"
#include "Model/DbModel.h"
#include "Utility/Constants.h"

RegisterPage::RegisterPage (QWidget *parent) : TempPage (parent)
{
    setWindowTitle (tr ("Register"));
    setWindowFlag (Qt::Window, true);
    setWindowModality (Qt::ApplicationModal);

    auto *centralWidget = new QWidget (nullptr);
    centralWidget->setWindowTitle (tr ("Register"));
    resize (858, 600);
    setMinimumSize (644, 450);
    setMaximumSize (644, 450);

    QVBoxLayout *registerPageLayout = new QVBoxLayout (centralWidget);
    registerPageLayout->setAlignment (Qt::AlignHCenter | Qt::AlignTop);
    registerPageLayout->setSpacing (20);
    registerPageLayout->setContentsMargins (15, 40, 15, 40);

    QLabel *titleLabel = new QLabel (tr ("Register User"), centralWidget);
    titleLabel->setAlignment (Qt::AlignHCenter);
    titleLabel->setStyleSheet (
        QString ("font-size: %1px; font-weight: bold; color: #333;")
            .arg (Constants::Settings::TITLE_FONT_SIZE));
    titleLabel->setFont (QFont (Constants::Settings::DEFAULT_FONT_FAMILY,
                                Constants::Settings::TITLE_FONT_SIZE,
                                QFont::Bold));

    registerPageLayout->addWidget (titleLabel);

    ElaLineEdit *usernameLineEdit = new ElaLineEdit (centralWidget);
    usernameLineEdit->setPlaceholderText (tr ("Username"));
    usernameLineEdit->setMinimumHeight (Constants::UI::DEFAULT_INPUT_HEIGHT);
    usernameLineEdit->setMaximumHeight (Constants::UI::DEFAULT_INPUT_HEIGHT);
    usernameLineEdit->setMinimumWidth (Constants::UI::DEFAULT_INPUT_WIDTH);
    usernameLineEdit->setMaximumWidth (Constants::UI::DEFAULT_INPUT_WIDTH);
    usernameLineEdit->setBorderRadius (Constants::UI::DEFAULT_BORDER_RADIUS);

    registerPageLayout->addWidget (usernameLineEdit);

    ElaLineEdit *passwordLineEdit = new ElaLineEdit (centralWidget);

    passwordLineEdit->setPlaceholderText (tr ("Password"));
    passwordLineEdit->setMinimumHeight (Constants::UI::DEFAULT_INPUT_HEIGHT);
    passwordLineEdit->setMaximumHeight (Constants::UI::DEFAULT_INPUT_HEIGHT);
    passwordLineEdit->setMinimumWidth (Constants::UI::DEFAULT_INPUT_WIDTH);
    passwordLineEdit->setMaximumWidth (Constants::UI::DEFAULT_INPUT_WIDTH);
    passwordLineEdit->setBorderRadius (Constants::UI::DEFAULT_BORDER_RADIUS);
    passwordLineEdit->setEchoMode (ElaLineEdit::Password);

    registerPageLayout->addWidget (passwordLineEdit);

    ElaLineEdit *confirmPasswordLineEdit = new ElaLineEdit (centralWidget);
    confirmPasswordLineEdit->setPlaceholderText (tr ("Confirm Password"));
    confirmPasswordLineEdit->setMinimumHeight (
        Constants::UI::DEFAULT_INPUT_HEIGHT);
    confirmPasswordLineEdit->setMaximumHeight (
        Constants::UI::DEFAULT_INPUT_HEIGHT);
    confirmPasswordLineEdit->setMinimumWidth (
        Constants::UI::DEFAULT_INPUT_WIDTH);
    confirmPasswordLineEdit->setMaximumWidth (
        Constants::UI::DEFAULT_INPUT_WIDTH);
    confirmPasswordLineEdit->setBorderRadius (
        Constants::UI::DEFAULT_BORDER_RADIUS);
    confirmPasswordLineEdit->setEchoMode (ElaLineEdit::Password);

    registerPageLayout->addWidget (confirmPasswordLineEdit);

    QHBoxLayout *registerButtonLayout = new QHBoxLayout (centralWidget);
    registerButtonLayout->setAlignment (Qt::AlignHCenter);
    registerButtonLayout->setSpacing (20);

    ElaPushButton *registerButton = new ElaPushButton (centralWidget);
    registerButton->setText (tr ("Register"));
    registerButton->setMinimumHeight (Constants::UI::DEFAULT_BUTTON_HEIGHT);
    registerButton->setMaximumHeight (Constants::UI::DEFAULT_BUTTON_HEIGHT);
    registerButton->setMinimumWidth (200);
    registerButton->setMaximumWidth (200);
    registerButton->setBorderRadius (Constants::UI::DEFAULT_BORDER_RADIUS);

    registerButtonLayout->addWidget (registerButton);

    ElaPushButton *cancelButton = new ElaPushButton (centralWidget);
    cancelButton->setText (tr ("Cancel"));
    cancelButton->setMinimumHeight (Constants::UI::DEFAULT_BUTTON_HEIGHT);
    cancelButton->setMaximumHeight (Constants::UI::DEFAULT_BUTTON_HEIGHT);
    cancelButton->setMinimumWidth (200);
    cancelButton->setMaximumWidth (200);
    cancelButton->setBorderRadius (Constants::UI::DEFAULT_BORDER_RADIUS);

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
                            tr ("Username or password cannot be empty.\n\n"
                                "Please enter your username/password."));
                return;
            }

            if (password != confirmPassword)
            {
                showDialog ("Register Error",
                            tr ("Passwords do not match.\n\n"
                                "Please confirm your password."));
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
                bool needRollback = false;
                QString rollbackReason;

                try
                {
                    // invoke UserModel to create user profile and settings
                    auto userDataResult =
                        UserModel::getInstance ().createUserData (
                            "profile_" +
                                AccountManager::getInstance ().getUserUuid (
                                    username) +
                                ".json",
                            username);

                    if (userDataResult != UserDataResult::Success)
                    {
                        needRollback = true;
                        auto it = UserDataResultMessage.find (userDataResult);
                        rollbackReason =
                            it != UserDataResultMessage.end ()
                                ? QString::fromStdString (it->second)
                                : "Unknown error creating user profile";
                    }
                }
                catch (const std::exception &e)
                {
                    needRollback = true;
                    rollbackReason = "Unexpected error: " +
                                     QString::fromStdString (e.what ());
                }
                catch (...)
                {
                    needRollback = true;
                    rollbackReason = "Unknown unexpected error occurred";
                }

                if (needRollback)
                {

                    try
                    {
                        DbModel::getInstance ().deleteUser (username);
                    }
                    catch (...)
                    {
                        rollbackReason +=
                            "\n" +
                            tr ("Failed to roll back user registration.");
                    }

                    showDialog (
                        tr ("Register Error"),
                        tr ("Registration failed during profile creation:") +
                            "\n" + rollbackReason + "\n\n" +
                            tr ("Registration has been rolled back."));
                    return;
                }

                showDialog (tr ("Register Success"),
                            tr ("User registered successfully.\n\nYou can now "
                                "log in with your username and password."));
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
