#include "RecitePage.h"
#include "ElaFlowLayout.h"
#include "ElaPushButton.h"
#include "Utility/Constants.h"
#include <qboxlayout.h>
#include <qlabel.h>


RecitePage::RecitePage (QWidget *parent) : TempPage (parent)
{
    setWindowTitle (tr ("Recite Page"));

    auto *centralWidget = new QWidget (this);
    centralWidget->setWindowTitle (tr ("Recite"));

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
    progressLabel->setText (tr ("Progress: %1/%2")
                                .arg (getProgress ().first)
                                .arg (getProgress ().second));
    progressLabel->setAlignment (Qt::AlignCenter);
    progressLabel->setStyleSheet (
        QString ("font-size: %1px; font-weight: bold; color: #333;")
            .arg (Constants::Settings::SUBTITLE_FONT_SIZE));
    progressLabel->setFont (QFont (Constants::Settings::DEFAULT_FONT_FAMILY,
                                   Constants::Settings::DEFAULT_FONT_SIZE,
                                   QFont::Normal));
    reciteWidgetLayout->addWidget (progressLabel);

    ElaPushButton *reciteButton =
        new ElaPushButton (tr ("Start Reciting"), reciteContentWidget);
    reciteButton->setFixedSize (180, 40);
    reciteButton->setStyleSheet (
        "QPushButton { background-color: #4CAF50; color: white; "
        "border-radius: 5px; }"
        "QPushButton:hover { background-color: #45a049; }");
    reciteButton->setFont (
        QFont (Constants::Settings::DEFAULT_FONT_FAMILY, 12, QFont::Normal));
    reciteWidgetLayout->addWidget (reciteButton);

    containerLayout->addWidget (reciteBgWidget, 0, 0);
    containerLayout->addWidget (reciteContentWidget, 0, 0);

    recitePageLayout->addWidget (containerWidget);

    QWidget *flowContainer = new QWidget (centralWidget);
    ElaFlowLayout *flowLayout = new ElaFlowLayout (flowContainer);
    flowLayout->setAlignment (Qt::AlignCenter | Qt::AlignTop);

    QWidget *favoritesWidget = new QWidget (flowContainer);
    favoritesWidget->setStyleSheet (
        "background-color: #FFF8DC; border-radius: 8px;");
    favoritesWidget->setFixedSize (600, 200);

    QLabel *favoritesLabel = new QLabel (favoritesWidget);
    favoritesLabel->setText (tr ("Favorites"));
    favoritesLabel->setStyleSheet (
        QString ("font-size: %1px; color: #333;")
            .arg (Constants::Settings::SUBTITLE_FONT_SIZE));
    QVBoxLayout *favoritesLayout = new QVBoxLayout (favoritesWidget);
    favoritesLayout->addWidget (favoritesLabel);
    favoritesLayout->addStretch ();

    QWidget *masteredWidget = new QWidget (flowContainer);
    masteredWidget->setStyleSheet (
        "background-color: #E0FFE0; border-radius: 8px;");
    masteredWidget->setFixedSize (600, 200);
    QLabel *masteredLabel = new QLabel (masteredWidget);
    masteredLabel->setText (tr ("Mastered"));
    masteredLabel->setStyleSheet (
        QString ("font-size: %1px; color: #333;")
            .arg (Constants::Settings::SUBTITLE_FONT_SIZE));

    QVBoxLayout *masteredLayout = new QVBoxLayout (masteredWidget);
    masteredLayout->addWidget (masteredLabel);
    masteredLayout->addStretch ();

    flowLayout->addWidget (favoritesWidget);
    flowLayout->addWidget (masteredWidget);

    recitePageLayout->addWidget (flowContainer);
    addCentralWidget (centralWidget, true, true, 0);
}