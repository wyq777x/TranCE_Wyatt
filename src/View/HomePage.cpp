#include "HomePage.h"
#include "ElaFlowLayout.h"
#include "ElaLineEdit.h"

HomePage::HomePage (QWidget *parent) : TempPage (parent)
{
    setWindowTitle ("Home");

    auto *centralWidget = new QWidget (this);
    centralWidget->setWindowTitle ("Home");

    ElaFlowLayout *layout = new ElaFlowLayout (centralWidget);
    ElaLineEdit *lineEdit = new ElaLineEdit (centralWidget);
    lineEdit->setPlaceholderText ("Search...");
    addCentralWidget (centralWidget, true, true, 0);
}