#include "TempPage.h"

TempPage::TempPage (QWidget *parent) : ElaScrollPage (parent)
{
    setWindowIcon (QIcon (Constants::Resources::APP_ICON));
};

TempPage::~TempPage () {}

// for operation result message display
void TempPage::showDialog (const QString &title, const QString &message)
{

    QDialog *Dialog = new QDialog (this);

    Dialog->setWindowTitle (title);
    Dialog->setWindowFlag (Qt::WindowStaysOnTopHint, true);
    Dialog->setWindowModality (Qt::ApplicationModal);
    Dialog->setModal (true);
    Dialog->setStyleSheet ("QDialog { background-color: #f0f0f0; "
                           "border-radius: 20px; }");

    Dialog->setMinimumSize (400, 250);
    Dialog->setMaximumSize (400, 250);

    Dialog->setAttribute (Qt::WA_DeleteOnClose, true);

    QVBoxLayout *Layout = new QVBoxLayout (Dialog);
    Layout->setSpacing (20);
    Layout->setContentsMargins (20, 20, 20, 20);

    QLabel *Label = new QLabel (message, Dialog);

    Label->setStyleSheet ("font-size: 16px; font-weight: bold; color: #333;");
    Label->setWordWrap (true);
    Label->setAlignment (Qt::AlignCenter);
    Label->setFont (QFont (Constants::Settings::DEFAULT_FONT_FAMILY,
                           Constants::Settings::DEFAULT_FONT_SIZE,
                           QFont::Bold));

    QDialogButtonBox *okButtonBox =
        new QDialogButtonBox (QDialogButtonBox::Ok, Dialog);

    okButtonBox->setStyleSheet (
        "QDialogButtonBox QPushButton { color: black; }"
        "QDialogButtonBox QPushButton:hover { color: #0078d7; }");

    Layout->addWidget (Label);
    Layout->addWidget (okButtonBox);
    Layout->setAlignment (okButtonBox, Qt::AlignHCenter);

    connect (okButtonBox, &QDialogButtonBox::accepted, Dialog,
             [=] ()
             {
                 Dialog->close ();
                 Dialog->deleteLater ();

                 QWidget *parentWidget = Dialog->parentWidget ();
                 if (parentWidget)
                 {
                     if (!title.contains (tr ("Error"), Qt::CaseInsensitive))
                     {
                         parentWidget->close ();
                     }
                 }
             });

    Dialog->exec ();
}

// for confirmation dialog
void TempPage::showDialog (const QString &title, const QString &message,
                           std::function<void ()> onConfirm,
                           std::function<void ()> onReject)
{
    QDialog *confirmDialog = new QDialog (this);
    confirmDialog->setWindowTitle (title);
    confirmDialog->setMinimumSize (300, 150);
    confirmDialog->setMaximumSize (300, 150);
    confirmDialog->setWindowModality (Qt::ApplicationModal);
    confirmDialog->setModal (true);
    confirmDialog->setStyleSheet ("QDialog { background-color: #f0f0f0; "
                                  "border-radius: 20px; }");

    QVBoxLayout *dialogLayout = new QVBoxLayout (confirmDialog);
    dialogLayout->setAlignment (Qt::AlignCenter);
    dialogLayout->setContentsMargins (20, 20, 20, 20);
    dialogLayout->setSpacing (20);

    QLabel *confirmLabel = new QLabel (message, confirmDialog);
    confirmLabel->setAlignment (Qt::AlignCenter);
    confirmLabel->setStyleSheet (
        "font-size: 16px; color: #333; font-weight: bold;");
    confirmLabel->setFont (QFont (Constants::Settings::DEFAULT_FONT_FAMILY,
                                  Constants::Settings::DEFAULT_FONT_SIZE,
                                  QFont::Bold));
    confirmLabel->setWordWrap (true);
    dialogLayout->addWidget (confirmLabel);

    QHBoxLayout *buttonLayout = new QHBoxLayout ();
    buttonLayout->setAlignment (Qt::AlignCenter);
    buttonLayout->setSpacing (20);

    ElaPushButton *yesButton = new ElaPushButton (tr ("Yes"), confirmDialog);
    yesButton->setMinimumHeight (50);
    yesButton->setMaximumHeight (50);
    yesButton->setMinimumWidth (120);
    yesButton->setMaximumWidth (120);
    yesButton->setBorderRadius (20);
    yesButton->setStyleSheet (
        "background-color: #4CAF50; color: white; font-size: "
        "16px; "
        "border-radius: 20px; padding: 10px;");
    yesButton->setFont (QFont (Constants::Settings::DEFAULT_FONT_FAMILY,
                               Constants::Settings::DEFAULT_FONT_SIZE));

    ElaPushButton *noButton = new ElaPushButton (tr ("No"), confirmDialog);
    noButton->setMinimumHeight (50);
    noButton->setMaximumHeight (50);
    noButton->setMinimumWidth (120);
    noButton->setMaximumWidth (120);
    noButton->setBorderRadius (20);
    noButton->setStyleSheet (
        "background-color: #F44336; color: white; font-size: "
        "16px; "
        "border-radius: 20px; padding: 10px;");
    noButton->setFont (QFont (Constants::Settings::DEFAULT_FONT_FAMILY,
                              Constants::Settings::DEFAULT_FONT_SIZE));

    buttonLayout->addWidget (yesButton);
    buttonLayout->addWidget (noButton);
    dialogLayout->addLayout (buttonLayout);

    connect (yesButton, &ElaPushButton::clicked, confirmDialog,
             [confirmDialog, onConfirm] ()
             {
                 if (onConfirm)
                 {
                     onConfirm ();
                 }
                 confirmDialog->close ();
                 auto *parentObj = confirmDialog->parentWidget ();
                 if (parentObj)
                 {
                     parentObj->close ();
                 }
                 confirmDialog->accept ();
             });

    connect (noButton, &ElaPushButton::clicked, confirmDialog,
             [confirmDialog, onReject] ()
             {
                 if (onReject)
                 {
                     onReject ();
                 }
                 confirmDialog->reject ();
             });

    if (confirmDialog->exec () == QDialog::Accepted)
    {
        qDebug () << "User confirmed action.";
    }
    else
    {
        qDebug () << "User canceled action.";
    }
}

// for changing password
void TempPage::showDialog (const QString &title,
                           std::function<void ()> onConfirm,
                           std::function<void ()> onReject)
{
    QDialog *changePasswordDialog = new QDialog (this);
    changePasswordDialog->setWindowTitle (title);
    changePasswordDialog->setMinimumSize (450, 350);
    changePasswordDialog->setMaximumSize (450, 350);
    changePasswordDialog->setWindowModality (Qt::ApplicationModal);
    changePasswordDialog->setModal (true);
    changePasswordDialog->setAttribute (Qt::WA_DeleteOnClose, true);
    changePasswordDialog->setStyleSheet ("QDialog { background-color: #f0f0f0; "
                                         "border-radius: 20px; }");

    QVBoxLayout *dialogLayout = new QVBoxLayout (changePasswordDialog);
    dialogLayout->setAlignment (Qt::AlignCenter);
    dialogLayout->setContentsMargins (30, 30, 30, 30);
    dialogLayout->setSpacing (20);

    // Title label
    QLabel *titleLabel = new QLabel (title, changePasswordDialog);
    titleLabel->setAlignment (Qt::AlignCenter);
    titleLabel->setStyleSheet (
        "font-size: 20px; color: #333; font-weight: bold;");
    titleLabel->setFont (QFont (Constants::Settings::DEFAULT_FONT_FAMILY,
                                Constants::Settings::SUBTITLE_FONT_SIZE,
                                QFont::Bold));
    dialogLayout->addWidget (titleLabel);

    // Old password input
    oldPasswordLineEdit = new ElaLineEdit (changePasswordDialog);
    oldPasswordLineEdit->setPlaceholderText (tr ("Old Password"));
    oldPasswordLineEdit->setMinimumHeight (50);
    oldPasswordLineEdit->setMaximumHeight (50);
    oldPasswordLineEdit->setMinimumWidth (350);
    oldPasswordLineEdit->setMaximumWidth (350);
    oldPasswordLineEdit->setBorderRadius (20);
    oldPasswordLineEdit->setEchoMode (ElaLineEdit::Password);
    dialogLayout->addWidget (oldPasswordLineEdit);

    // New password input
    newPasswordLineEdit = new ElaLineEdit (changePasswordDialog);
    newPasswordLineEdit->setPlaceholderText (tr ("New Password"));
    newPasswordLineEdit->setMinimumHeight (50);
    newPasswordLineEdit->setMaximumHeight (50);
    newPasswordLineEdit->setMinimumWidth (350);
    newPasswordLineEdit->setMaximumWidth (350);
    newPasswordLineEdit->setBorderRadius (20);
    newPasswordLineEdit->setEchoMode (ElaLineEdit::Password);
    dialogLayout->addWidget (newPasswordLineEdit);

    // Button layout
    QHBoxLayout *buttonLayout = new QHBoxLayout ();
    buttonLayout->setAlignment (Qt::AlignCenter);
    buttonLayout->setSpacing (20);

    ElaPushButton *confirmButton =
        new ElaPushButton (tr ("Modify"), changePasswordDialog);
    confirmButton->setMinimumHeight (50);
    confirmButton->setMaximumHeight (50);
    confirmButton->setMinimumWidth (120);
    confirmButton->setMaximumWidth (120);
    confirmButton->setBorderRadius (20);
    confirmButton->setStyleSheet (
        "background-color: #4CAF50; color: white; font-size: "
        "16px; "
        "border-radius: 20px; padding: 10px;");
    confirmButton->setFont (QFont (Constants::Settings::DEFAULT_FONT_FAMILY,
                                   Constants::Settings::DEFAULT_FONT_SIZE));

    ElaPushButton *cancelButton =
        new ElaPushButton (tr ("Cancel"), changePasswordDialog);
    cancelButton->setMinimumHeight (50);
    cancelButton->setMaximumHeight (50);
    cancelButton->setMinimumWidth (120);
    cancelButton->setMaximumWidth (120);
    cancelButton->setBorderRadius (20);
    cancelButton->setStyleSheet (
        "background-color: #F44336; color: white; font-size: "
        "16px; "
        "border-radius: 20px; padding: 10px;");
    cancelButton->setFont (QFont (Constants::Settings::DEFAULT_FONT_FAMILY,
                                  Constants::Settings::DEFAULT_FONT_SIZE));

    buttonLayout->addWidget (confirmButton);
    buttonLayout->addWidget (cancelButton);
    dialogLayout->addLayout (buttonLayout);

    connect (confirmButton, &ElaPushButton::clicked, changePasswordDialog,
             [changePasswordDialog, onConfirm] ()
             {
                 if (onConfirm)
                 {
                     onConfirm ();
                 }
                 changePasswordDialog->accept ();
             });

    connect (cancelButton, &ElaPushButton::clicked, changePasswordDialog,
             [changePasswordDialog, onReject] ()
             {
                 if (onReject)
                 {
                     onReject ();
                 }
                 changePasswordDialog->reject ();
             });

    changePasswordDialog->exec ();
}