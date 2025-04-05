#include "MyPage.h"

MyPage::MyPage (QWidget *parent) : TempPage (parent)
{
    setWindowTitle ("My Page");

    auto *centralWidget = new QWidget (this);
    centralWidget->setWindowTitle ("My Page");

    addCentralWidget (centralWidget, true, true, 0);
}