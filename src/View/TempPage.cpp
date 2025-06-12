#include "TempPage.h"

TempPage::TempPage (QWidget *parent) : ElaScrollPage (parent)
{

    setWindowIcon (QIcon (":image/learnENG.ico"));
};

TempPage::~TempPage () {}

// for operation result message display
void TempPage::showDialog (const QString &title, const QString &message)
{

    QDialog *Dialog = new QDialog (this);

    Dialog->setWindowTitle (title);
    Dialog->setWindowModality (Qt::ApplicationModal);
    Dialog->setModal (true);

    Dialog->setMinimumSize (400, 250);
    Dialog->setMaximumSize (400, 250);

    Dialog->setAttribute (Qt::WA_DeleteOnClose, true);

    QVBoxLayout *Layout = new QVBoxLayout (Dialog);
    Layout->setSpacing (20);

    QLabel *Label = new QLabel (message, Dialog);

    Label->setStyleSheet ("font-size: 16px; font-weight: bold; color: #333;");
    Label->setWordWrap (true);
    Label->setAlignment (Qt::AlignCenter);

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
                     if (!title.contains ("Error", Qt::CaseInsensitive))
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
    confirmDialog->setStyleSheet ("QDialog { background-color: #f0f0f0; "
                                  "border-radius: 10px; }");

    QVBoxLayout *dialogLayout = new QVBoxLayout (confirmDialog);
    dialogLayout->setAlignment (Qt::AlignCenter);
    dialogLayout->setContentsMargins (20, 20, 20, 20);

    QLabel *confirmLabel = new QLabel (message, confirmDialog);
    confirmLabel->setAlignment (Qt::AlignCenter);
    confirmLabel->setStyleSheet (
        "font-size: 16px; color: #333; font-weight: bold;");
    confirmLabel->setFont (QFont ("Noto Sans", 16, QFont::Bold));
    dialogLayout->addWidget (confirmLabel);

    QHBoxLayout *buttonLayout = new QHBoxLayout ();
    ElaPushButton *yesButton = new ElaPushButton ("Yes", confirmDialog);
    yesButton->setStyleSheet (
        "background-color: #4CAF50; color: white; font-size: "
        "16px; "
        "border-radius: 5px; padding: 10px;");
    yesButton->setFont (QFont ("Noto Sans", 16));

    ElaPushButton *noButton = new ElaPushButton ("No", confirmDialog);
    noButton->setStyleSheet (
        "background-color: #F44336; color: white; font-size: "
        "16px; "
        "border-radius: 5px; padding: 10px;");
    noButton->setFont (QFont ("Noto Sans", 16));

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
                                         "border-radius: 10px; }");

    QVBoxLayout *dialogLayout = new QVBoxLayout (changePasswordDialog);
    dialogLayout->setAlignment (Qt::AlignCenter);
    dialogLayout->setContentsMargins (30, 30, 30, 30);
    dialogLayout->setSpacing (20);

    // Title label
    QLabel *titleLabel = new QLabel (title, changePasswordDialog);
    titleLabel->setAlignment (Qt::AlignCenter);
    titleLabel->setStyleSheet (
        "font-size: 20px; color: #333; font-weight: bold;");
    titleLabel->setFont (QFont ("Noto Sans", 20, QFont::Bold));
    dialogLayout->addWidget (titleLabel);

    // Old password input
    ElaLineEdit *oldPasswordLineEdit = new ElaLineEdit (changePasswordDialog);
    oldPasswordLineEdit->setPlaceholderText ("Old Password");
    oldPasswordLineEdit->setMinimumHeight (50);
    oldPasswordLineEdit->setMaximumHeight (50);
    oldPasswordLineEdit->setMinimumWidth (350);
    oldPasswordLineEdit->setMaximumWidth (350);
    oldPasswordLineEdit->setBorderRadius (20);
    oldPasswordLineEdit->setEchoMode (ElaLineEdit::Password);
    dialogLayout->addWidget (oldPasswordLineEdit);

    // New password input
    ElaLineEdit *newPasswordLineEdit = new ElaLineEdit (changePasswordDialog);
    newPasswordLineEdit->setPlaceholderText ("New Password");
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
        new ElaPushButton ("Modify", changePasswordDialog);
    confirmButton->setMinimumHeight (50);
    confirmButton->setMaximumHeight (50);
    confirmButton->setMinimumWidth (120);
    confirmButton->setMaximumWidth (120);
    confirmButton->setBorderRadius (20);
    confirmButton->setStyleSheet (
        "background-color: #4CAF50; color: white; font-size: "
        "16px; "
        "border-radius: 20px; padding: 10px;");
    confirmButton->setFont (QFont ("Noto Sans", 16));

    ElaPushButton *cancelButton =
        new ElaPushButton ("Cancel", changePasswordDialog);
    cancelButton->setMinimumHeight (50);
    cancelButton->setMaximumHeight (50);
    cancelButton->setMinimumWidth (120);
    cancelButton->setMaximumWidth (120);
    cancelButton->setBorderRadius (20);
    cancelButton->setStyleSheet (
        "background-color: #F44336; color: white; font-size: "
        "16px; "
        "border-radius: 20px; padding: 10px;");
    cancelButton->setFont (QFont ("Noto Sans", 16));

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