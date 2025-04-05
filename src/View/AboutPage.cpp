#include "AboutPage.h"
#include "ElaFlowLayout.h"
#include <qboxlayout.h>
#include <qgridlayout.h>
#include <qlabel.h>

AboutPage::AboutPage (QWidget *parent) : TempPage (parent)
{
    setWindowTitle ("About");

    auto *centralWidget = new QWidget (this);
    centralWidget->setWindowTitle ("About");

    QGridLayout *layout = new QGridLayout (centralWidget);

    QLabel *titleLabel = new QLabel ("TranCE_Wyatt", centralWidget);
    titleLabel->setStyleSheet (
        "font-size: 24px; font-weight: bold; color: #333;");
    titleLabel->setFont (QFont ("Noto Sans", 24, QFont::Bold));
    titleLabel->setAlignment (Qt::AlignCenter);

    titleLabel->setFixedHeight (50);

    layout->addWidget (titleLabel, 0, 1, Qt::AlignCenter);

    layout->setRowStretch (1, 1);
    layout->setColumnStretch (0, 1);
    layout->setColumnStretch (2, 1);
    addCentralWidget (centralWidget, true, true, 0);
}