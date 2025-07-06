#include "HomePage.h"
#include "DbManager.h"
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

    LangComboBox_left->setToolTip ("Select Source Language");

    ElaComboBox *LangComboBox_right = new ElaComboBox (centralWidget);

    LangComboBox_right->setToolTip ("Select Target Language");

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
    swapButton->setToolTip ("Swap Languages");

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

    searchOnline->setToolTip ("Search Online by Bing");

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

    ElaListView *suggestionsList = new ElaListView (centralWidget);

    suggestionsList->setEditTriggers (QAbstractItemView::NoEditTriggers);
    suggestionsList->setIsTransparent (true);
    suggestionsList->setFixedSize (550, 200);

    suggestionsList->hide ();

    QStringListModel *suggestionModel = new QStringListModel (this);
    suggestionsList->setModel (suggestionModel);

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

    QHBoxLayout *searchModeLayout = new QHBoxLayout (centralWidget);
    searchModeLayout->setAlignment (Qt::AlignHCenter);
    searchModeLayout->setSpacing (20);
    ElaRadioButton *searchMode_precise =
        new ElaRadioButton ("Precise Search", centralWidget);
    searchMode_precise->setToolTip ("Precise Search Mode");

    ElaRadioButton *searchMode_fuzzy =
        new ElaRadioButton ("Fuzzy Search", centralWidget);
    searchMode_fuzzy->setToolTip ("Fuzzy Search Mode");
    searchMode_fuzzy->setChecked (true);

    searchModeLayout->addWidget (searchMode_precise);
    searchModeLayout->addWidget (searchMode_fuzzy);

    homePageLayout->addLayout (searchModeLayout);

    homePageLayout->addWidget (suggestionsList);

    QLabel *randomRecommendationLabel =
        new QLabel ("Random Recommendation:", centralWidget);
    randomRecommendationLabel->setStyleSheet (
        "font-size: 20px; font-weight: normal; color: #333;");
    randomRecommendationLabel->setFont (QFont ("Noto Sans", 24));

    homePageLayout->addWidget (randomRecommendationLabel);

    // Random Recommendation Layout
    QHBoxLayout *randomRecommendationLayout = new QHBoxLayout (centralWidget);
    randomRecommendationLayout->setAlignment (Qt::AlignHCenter);
    randomRecommendationLayout->setSpacing (20);

    ElaPushButton *recommendWordButton =
        new ElaPushButton ("Recommend Word", centralWidget);
    recommendWordButton->setMinimumHeight (50);
    recommendWordButton->setMinimumWidth (600);
    recommendWordButton->setBorderRadius (8);
    recommendWordButton->setToolTip ("Get a random word recommendation");

    randomRecommendationLayout->addWidget (recommendWordButton);

    homePageLayout->addLayout (randomRecommendationLayout);

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

                 suggestionModel->setStringList (QStringList ());
                 suggestionsList->hide ();
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
             });

    connect (clearAction, &QAction::triggered, [=] () { lineEdit->clear (); });

    connect (lineEdit, &ElaLineEdit::textChanged,
             [=, this] (const QString &text)
             {
                 if (text.isEmpty ())
                 {
                     clearAction->setVisible (false);
                     suggestionsList->hide ();
                 }
                 else
                 {
                     clearAction->setVisible (true);

                     if (LangComboBox_left->currentText () ==
                         LangComboBox_right->currentText ())
                     {
                         suggestionsList->hide ();
                         return;
                     }

                     if (LangComboBox_left->currentText ().isEmpty () ||
                         LangComboBox_right->currentText ().isEmpty ())
                     {
                         suggestionsList->hide ();
                         return;
                     }

                     if (LangComboBox_left->currentText () == "English" &&
                         LangComboBox_right->currentText () == "Chinese")
                     {
                         if (searchMode_precise->isChecked ())
                         {
                             auto wordEntry =
                                 DbManager::getInstance ().lookupWord (
                                     lineEdit->text (),
                                     LangComboBox_left->currentText ());

                             QStringList singleSuggestion;
                             singleSuggestion << wordEntry->word;
                             suggestionModel->setStringList (singleSuggestion);
                             suggestionsList->setModel (suggestionModel);
                         }

                         if (searchMode_fuzzy->isChecked ())
                         {

                             // to be implemented
                         }
                     }
                     else if (LangComboBox_left->currentText () == "Chinese" &&
                              LangComboBox_right->currentText () == "English")
                     {
                         suggestionsList->setModel (suggestionModel);
                         suggestionsList->hide ();
                         return;
                     }
                     else
                     {
                         suggestionsList->hide ();
                         return;
                     }
                     suggestionsList->show ();
                 }
             });

    connect (suggestionsList, &ElaListView::clicked, this,
             [=, this] (const QModelIndex &index)
             {
                 // Building...

                 if (searchOnline->getIsToggled ())
                 {
                     QDesktopServices::openUrl (
                         QUrl (QString ("https://www.bing.com/search?q=%1")
                                   .arg (index.data ().toString ())));
                     return;
                 }
                 auto wordEntry = DbManager::getInstance ().lookupWord (
                     index.data ().toString (),
                     LangComboBox_left->currentText () == "English" ? "en"
                                                                    : "zh");
                 auto wordCard = WordCard::getInstance ();

                 wordCard->setWordEntry (wordEntry.value ());

                 wordCard->show ();
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

    connect (recommendWordButton, &ElaPushButton::clicked,
             [=] () {

             });

    addCentralWidget (centralWidget, true, true, 0);
}