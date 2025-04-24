#include "RecitePage.h"

RecitePage::RecitePage (QWidget *parent) : TempPage (parent)
{
    setWindowTitle ("Recite Page");

    auto *centralWidget = new QWidget (this);
    centralWidget->setWindowTitle ("Recite");

    QVBoxLayout *recitePageLayout = new QVBoxLayout (centralWidget);
    recitePageLayout->setAlignment (Qt::AlignHCenter | Qt::AlignTop);
    recitePageLayout->setSizeConstraint (QLayout::SetMinimumSize);

    QWidget *reciteWidget = new QWidget (centralWidget);
    QGraphicsBlurEffect *blurEffect = new QGraphicsBlurEffect (reciteWidget);
    blurEffect->setBlurRadius (5);
    reciteWidget->setGraphicsEffect (blurEffect);
    reciteWidget->setStyleSheet (
        "QWidget { background-color: rgba(175, 238, 255, 0.8); "
        "border-radius: 10px; }");
    reciteWidget->setMinimumWidth (600);
    reciteWidget->setMaximumWidth (600);
    reciteWidget->setMinimumHeight (200);
    reciteWidget->setMaximumHeight (200);

    recitePageLayout->addWidget (reciteWidget);
    addCentralWidget (centralWidget, true, true, 0);
}