#include "LoginPage.h"
#include "ElaLineEdit.h"
#include "ElaPushButton.h"
#include <qnamespace.h>
LoginPage::LoginPage (QWidget *parent) : TempPage (parent)
{
    setWindowTitle ("Login");
    setWindowFlag (Qt::WindowStaysOnTopHint, true);
    setWindowModality (Qt::ApplicationModal);

    auto *centralWidget = new QWidget (this);
    centralWidget->setWindowTitle ("Login");
    resize (858, 600); // 1.43:1
    setMinimumSize (858, 600);
    setMaximumSize (858, 600);

    ElaLineEdit *usernameLineEdit = new ElaLineEdit (centralWidget);
    usernameLineEdit->setPlaceholderText ("Username");
    usernameLineEdit->setGeometry (100, 100, 200, 40);

    ElaLineEdit *passwordLineEdit = new ElaLineEdit (centralWidget);
    passwordLineEdit->setPlaceholderText ("Password");
    passwordLineEdit->setGeometry (100, 160, 200, 40);
    passwordLineEdit->setEchoMode (ElaLineEdit::Password);

    ElaPushButton *loginButton = new ElaPushButton (centralWidget);
    loginButton->setText ("Login");
    loginButton->setGeometry (100, 220, 200, 40);

    ElaPushButton *registerButton = new ElaPushButton (centralWidget);
    registerButton->setText ("Register");
    registerButton->setGeometry (100, 280, 200, 40);

    addCentralWidget (centralWidget, true, true, 0);
}

void LoginPage::LoginPage::paintEvent (QPaintEvent *event)
{
    Q_UNUSED (event);
    QPainter painter (this);
    painter.setRenderHint (QPainter::Antialiasing, true);
    painter.setRenderHint (QPainter::TextAntialiasing, true);
    painter.setRenderHint (QPainter::SmoothPixmapTransform, true);

    painter.fillRect (rect (), QBrush (Qt::white));

    painter.setPen (Qt::black);
}