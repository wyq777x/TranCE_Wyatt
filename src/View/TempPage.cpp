#include "TempPage.h"

TempPage::TempPage (QWidget *parent) : ElaScrollPage (parent)
{

    setWindowIcon (QIcon (":res/image/learnENG.ico"));
};

TempPage::~TempPage () {}

void TempPage::showErrorDialog (const QString &title, const QString &message)
{
    QDialog *errorDialog = new QDialog (this);

    errorDialog->setWindowTitle (title);
    errorDialog->setWindowModality (Qt::ApplicationModal);
    errorDialog->setModal (true);

    errorDialog->setMinimumSize (400, 250);
    errorDialog->setMaximumSize (400, 250);

    errorDialog->setAttribute (Qt::WA_DeleteOnClose, true);

    QVBoxLayout *errorLayout = new QVBoxLayout (errorDialog);
    errorLayout->setSpacing (20);

    QLabel *errorLabel = new QLabel (message, errorDialog);

    errorLabel->setStyleSheet (
        "font-size: 16px; font-weight: bold; color: #333;");
    errorLabel->setWordWrap (true);
    errorLabel->setAlignment (Qt::AlignCenter);

    QDialogButtonBox *okButtonBox =
        new QDialogButtonBox (QDialogButtonBox::Ok, errorDialog);

    okButtonBox->setStyleSheet (
        "QDialogButtonBox QPushButton { color: black; }"
        "QDialogButtonBox QPushButton:hover { color: #0078d7; }");

    errorLayout->addWidget (errorLabel);
    errorLayout->addWidget (okButtonBox);
    errorLayout->setAlignment (okButtonBox, Qt::AlignHCenter);

    connect (okButtonBox, &QDialogButtonBox::accepted, errorDialog,
             &QDialog::accept);

    errorDialog->exec ();
}