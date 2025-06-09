#include "TempPage.h"

TempPage::TempPage (QWidget *parent) : ElaScrollPage (parent)
{

    setWindowIcon (QIcon (":res/image/learnENG.ico"));
};

TempPage::~TempPage () {}

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