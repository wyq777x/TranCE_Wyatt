#include "MyPage.h"
#include <qnamespace.h>

MyPage::MyPage (QWidget *parent) : TempPage (parent)
{
    setWindowTitle ("My Page");

    auto *centralWidget = new QWidget (this);
    centralWidget->setWindowTitle ("My Page");

    QVBoxLayout *userProfileLayout = new QVBoxLayout (centralWidget);
    userProfileLayout->setAlignment (Qt::AlignCenter);

    QHBoxLayout *avatarLayout = new QHBoxLayout (centralWidget);

    addCentralWidget (centralWidget, true, true, 0);
}