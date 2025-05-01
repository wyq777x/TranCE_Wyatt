#include "HomePage.h"
#include "Def.h"
#include "ElaComboBox.h"
#include "ElaFlowLayout.h"
#include "ElaIcon.h"
#include "ElaLineEdit.h"
#include "ElaPushButton.h"
#include "ElaToggleButton.h"
#include <qboxlayout.h>
#include <qnamespace.h>

HomePage::HomePage (QWidget *parent) : TempPage (parent)
{
    setWindowTitle ("Home");

    auto *centralWidget = new QWidget (this);
    centralWidget->setWindowTitle ("Home");

    QVBoxLayout *homePageLayout = new QVBoxLayout (centralWidget);
    homePageLayout->setAlignment (Qt::AlignHCenter | Qt::AlignTop);
    homePageLayout->setSpacing (20);

    ElaComboBox *LangComboBox_left = new ElaComboBox (centralWidget);
    ElaComboBox *LangComboBox_right = new ElaComboBox (centralWidget);
    LangComboBox_left->setStyleSheet (
        "QComboBox {"
        "    color: black;" // set default text color as black
        "    font-family: 'Noto Sans';"
        "}"
        "QComboBox QAbstractItemView {"
        "    color: black;" // set text color of dropdown list as black
        "}");

    LangComboBox_right->setStyleSheet (
        "QComboBox {"
        "    color: black;" // set default text color as black
        "    font-family: 'Noto Sans';"
        "}"
        "QComboBox QAbstractItemView {"
        "    color: black;" // set text color of dropdown list as black
        "}");

    ElaIconButton *swapButton =
        new ElaIconButton (ElaIconType::SwapArrows, centralWidget);
    swapButton->setBorderRadius (3);

    QHBoxLayout *langLayout = new QHBoxLayout (centralWidget);
    langLayout->setAlignment (Qt::AlignHCenter);
    langLayout->setSpacing (20);
    langLayout->addWidget (LangComboBox_left);
    langLayout->addWidget (swapButton);
    langLayout->addWidget (LangComboBox_right);

    homePageLayout->addLayout (langLayout);
    homePageLayout->setAlignment (langLayout, Qt::AlignHCenter);

    LangComboBox_left->addItem ("English");
    LangComboBox_left->addItem ("Chinese");

    LangComboBox_right->addItem ("Chinese");
    LangComboBox_right->addItem ("English");

    ElaToggleButton *searchOnline = new ElaToggleButton (centralWidget);
    searchOnline->setText (R"(Search
Online)");
    searchOnline->setMinimumHeight (60);
    searchOnline->setMaximumHeight (60);
    searchOnline->setBorderRadius (6);
    searchOnline->setIsToggled (false); // default is false

    ElaLineEdit *lineEdit = new ElaLineEdit (centralWidget);
    lineEdit->setPlaceholderText ("Search...");
    lineEdit->setMinimumHeight (70);
    lineEdit->setMaximumHeight (70);
    lineEdit->setMinimumWidth (550);
    lineEdit->setBorderRadius (20);

    QAction *searchAction = lineEdit->addAction (
        ElaIcon::getInstance ()->getElaIcon (ElaIconType::MagnifyingGlass, 512),
        QLineEdit::LeadingPosition);
    searchAction->setToolTip ("Search");

    QAction *clearAction = lineEdit->addAction (
        ElaIcon::getInstance ()->getElaIcon (ElaIconType::Xmark, 512),
        QLineEdit::TrailingPosition);
    clearAction->setToolTip ("Clear");
    clearAction->setVisible (false);

    QHBoxLayout *searchLayout = new QHBoxLayout (centralWidget);
    searchLayout->setAlignment (Qt::AlignHCenter);
    searchLayout->setSpacing (20);
    searchLayout->addWidget (lineEdit);
    searchLayout->addWidget (searchOnline);
    homePageLayout->addLayout (searchLayout);

    QLabel *randomRecommendationLabel =
        new QLabel ("Random Recommendation:", centralWidget);
    randomRecommendationLabel->setStyleSheet (
        "font-size: 20px; font-weight: normal; color: #333;");
    randomRecommendationLabel->setFont (QFont ("Noto Sans", 24));

    homePageLayout->addWidget (randomRecommendationLabel);

    connect (swapButton, &ElaIconButton::clicked,
             [=] ()
             {
                 QString leftText = LangComboBox_left->currentText ();
                 QString rightText = LangComboBox_right->currentText ();

                 LangComboBox_left->blockSignals (true);
                 LangComboBox_right->blockSignals (true);

                 int leftIndex = LangComboBox_right->findText (leftText);
                 int rightIndex = LangComboBox_left->findText (rightText);

                 if (leftIndex >= 0 && rightIndex >= 0)
                 {
                     LangComboBox_left->setCurrentIndex (rightIndex);
                     LangComboBox_right->setCurrentIndex (leftIndex);
                 }

                 LangComboBox_left->blockSignals (false);
                 LangComboBox_right->blockSignals (false);

                 // emit languageSwapped
             });

    connect (searchAction, &QAction::triggered,
             [=] ()
             {
                 if (searchOnline->getIsToggled ())
                 {
                     QDesktopServices::openUrl (
                         QUrl (QString ("https://www.bing.com/search?q=%1")
                                   .arg (lineEdit->text ())));
                     return;
                 }

                 // Query the local database
                 // emit searchTriggered
             });

    connect (clearAction, &QAction::triggered, [=] () { lineEdit->clear (); });

    connect (lineEdit, &ElaLineEdit::textChanged,
             [=] (const QString &text)
             {
                 if (text.isEmpty ())
                 {
                     clearAction->setVisible (false);
                 }
                 else
                 {
                     clearAction->setVisible (true);
                 }
             });
    connect (lineEdit, &ElaLineEdit::returnPressed,
             [=] ()
             {
                 if (searchOnline->getIsToggled ())
                 {
                     if (lineEdit->text ().isEmpty ())
                     {
                         return;
                     }
                     QDesktopServices::openUrl (
                         QUrl (QString ("https://www.bing.com/search?q=%1")
                                   .arg (lineEdit->text ())));
                     return;
                 }
             });

    addCentralWidget (centralWidget, true, true, 0);
}