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