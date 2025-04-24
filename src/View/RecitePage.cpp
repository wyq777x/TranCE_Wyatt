#include "RecitePage.h"
#include "ElaPushButton.h"

RecitePage::RecitePage (QWidget *parent) : TempPage (parent)
{
    setWindowTitle ("Recite Page");

    auto *centralWidget = new QWidget (this);
    centralWidget->setWindowTitle ("Recite");

    QVBoxLayout *recitePageLayout = new QVBoxLayout (centralWidget);
    recitePageLayout->setAlignment (Qt::AlignHCenter | Qt::AlignTop);
    recitePageLayout->setSizeConstraint (QLayout::SetMinimumSize);

    QWidget *containerWidget = new QWidget (centralWidget);
    containerWidget->setFixedSize (600, 200);
    QGridLayout *containerLayout = new QGridLayout (containerWidget);
    containerLayout->setContentsMargins (0, 0, 0, 0);
    containerLayout->setSpacing (0);

    QWidget *reciteBgWidget = new QWidget ();
    QGraphicsBlurEffect *blurEffect = new QGraphicsBlurEffect (reciteBgWidget);
    blurEffect->setBlurRadius (5);
    reciteBgWidget->setGraphicsEffect (blurEffect);
    reciteBgWidget->setStyleSheet (
        "QWidget { background-color: rgba(175, 238, 255, 0.8); "
        "border-radius: 10px; }");
    reciteBgWidget->setFixedSize (600, 200);

    QWidget *reciteContentWidget = new QWidget ();
    reciteContentWidget->setStyleSheet ("background-color: transparent;");
    QVBoxLayout *reciteWidgetLayout = new QVBoxLayout (reciteContentWidget);
    reciteWidgetLayout->setAlignment (Qt::AlignCenter);

    QLabel *progressLabel = new QLabel (reciteContentWidget);
    progressLabel->setText (QString ("Progress: %1/%2")
                                .arg (getProgress ().first)
                                .arg (getProgress ().second));
    progressLabel->setAlignment (Qt::AlignCenter);
    progressLabel->setStyleSheet (
        "font-size: 20px; font-weight: bold; color: #333;");
    progressLabel->setFont (QFont ("Noto Sans", 16, QFont::Normal));
    reciteWidgetLayout->addWidget (progressLabel);

    ElaPushButton *reciteButton =
        new ElaPushButton ("Start Reciting", reciteContentWidget);
    reciteButton->setFixedSize (180, 40);
    reciteButton->setStyleSheet (
        "QPushButton { background-color: #4CAF50; color: white; "
        "border-radius: 5px; }"
        "QPushButton:hover { background-color: #45a049; }");
    reciteButton->setFont (QFont ("Noto Sans", 12, QFont::Normal));
    reciteWidgetLayout->addWidget (reciteButton);

    containerLayout->addWidget (reciteBgWidget, 0, 0);
    containerLayout->addWidget (reciteContentWidget, 0, 0);

    recitePageLayout->addWidget (containerWidget);
    addCentralWidget (centralWidget, true, true, 0);
}