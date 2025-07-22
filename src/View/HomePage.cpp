#include "HomePage.h"
#include "Controller/DbManager.h"
#include "Utility/Constants.h"
#include "View/Components/WordCard.h"

HomePage::HomePage (QWidget *parent) : TempPage (parent)
{
    setWindowTitle (tr ("Home"));

    initUI ();
    initConnections ();
}

void HomePage::initUI ()
{
    centralWidget = new QWidget (this);
    centralWidget->setWindowTitle (tr ("Home"));

    homePageLayout = new QVBoxLayout (centralWidget);
    homePageLayout->setAlignment (Qt::AlignHCenter | Qt::AlignTop);
    homePageLayout->setSpacing (20);

    LangComboBox_left = new ElaComboBox (centralWidget);
    LangComboBox_left->setToolTip (tr ("Select Source Language"));

    LangComboBox_right = new ElaComboBox (centralWidget);
    LangComboBox_right->setToolTip (tr ("Select Target Language"));

    LangComboBox_left->setStyleSheet (
        "QComboBox {"
        "    color: black;" // set default text color as black
        "    font-family: '" +
        Constants::Settings::DEFAULT_FONT_FAMILY +
        "';"
        "}"
        "QComboBox QAbstractItemView {"
        "    color: black;" // set text color of dropdown list as black
        "}");

    LangComboBox_right->setStyleSheet (
        "QComboBox {"
        "    color: black;" // set default text color as black
        "    font-family: '" +
        Constants::Settings::DEFAULT_FONT_FAMILY +
        "';"
        "}"
        "QComboBox QAbstractItemView {"
        "    color: black;" // set text color of dropdown list as black
        "}");

    swapButton = new ElaIconButton (ElaIconType::SwapArrows, centralWidget);
    swapButton->setBorderRadius (Constants::UI::SMALL_BORDER_RADIUS);
    swapButton->setToolTip (tr ("Swap Languages"));

    langLayout = new QHBoxLayout (centralWidget);
    langLayout->setAlignment (Qt::AlignHCenter);
    langLayout->setSpacing (20);
    langLayout->addWidget (LangComboBox_left);
    langLayout->addWidget (swapButton);
    langLayout->addWidget (LangComboBox_right);

    homePageLayout->addLayout (langLayout);
    homePageLayout->setAlignment (langLayout, Qt::AlignHCenter);

    LangComboBox_left->addItem (tr ("English"));
    LangComboBox_left->addItem (tr ("Chinese"));

    LangComboBox_right->addItem (tr ("Chinese"));
    LangComboBox_right->addItem (tr ("English"));

    searchOnline = new ElaToggleButton (centralWidget);
    searchOnline->setToolTip (tr ("Search Online by Bing"));
    searchOnline->setText (tr ("Search\nOnline"));
    searchOnline->setMinimumHeight (60);
    searchOnline->setMaximumHeight (60);
    searchOnline->setBorderRadius (Constants::UI::SMALL_BORDER_RADIUS);
    searchOnline->setIsToggled (false); // default is false

    lineEdit = new ElaLineEdit (centralWidget);
    lineEdit->setPlaceholderText (tr ("Search..."));
    lineEdit->setMinimumHeight (70);
    lineEdit->setMaximumHeight (70);
    lineEdit->setMinimumWidth (550);
    lineEdit->setBorderRadius (Constants::UI::DEFAULT_BORDER_RADIUS);

    suggestionsList = new ElaListView (centralWidget);
    suggestionsList->setEditTriggers (QAbstractItemView::NoEditTriggers);
    suggestionsList->setIsTransparent (true);
    suggestionsList->setFixedSize (550, 200);
    suggestionsList->hide ();

    suggestionModel = new QStringListModel (this);
    suggestionsList->setModel (suggestionModel);

    searchAction = lineEdit->addAction (
        ElaIcon::getInstance ()->getElaIcon (ElaIconType::MagnifyingGlass, 512),
        QLineEdit::LeadingPosition);
    searchAction->setToolTip (tr ("Search"));

    clearAction = lineEdit->addAction (
        ElaIcon::getInstance ()->getElaIcon (ElaIconType::Xmark, 512),
        QLineEdit::TrailingPosition);
    clearAction->setToolTip (tr ("Clear"));
    clearAction->setVisible (false);

    searchLayout = new QHBoxLayout (centralWidget);
    searchLayout->setAlignment (Qt::AlignHCenter);
    searchLayout->setSpacing (20);
    searchLayout->addWidget (lineEdit);
    searchLayout->addWidget (searchOnline);

    homePageLayout->addLayout (searchLayout);

    searchModeLayout = new QHBoxLayout (centralWidget);
    searchModeLayout->setAlignment (Qt::AlignHCenter);
    searchModeLayout->setSpacing (20);

    searchMode_precise =
        new ElaRadioButton (tr ("Precise Search"), centralWidget);
    searchMode_precise->setToolTip (tr ("Precise Search Mode"));

    searchMode_fuzzy = new ElaRadioButton (tr ("Fuzzy Search"), centralWidget);
    searchMode_fuzzy->setToolTip (tr ("Fuzzy Search Mode"));
    searchMode_fuzzy->setChecked (true);

    searchModeLayout->addWidget (searchMode_precise);
    searchModeLayout->addWidget (searchMode_fuzzy);

    homePageLayout->addLayout (searchModeLayout);

    homePageLayout->addWidget (suggestionsList);

    randomRecommendationLabel =
        new QLabel (tr ("Random Recommendation:"), centralWidget);
    randomRecommendationLabel->setStyleSheet (
        "font-size: 20px; font-weight: normal; color: #333;");
    randomRecommendationLabel->setFont (
        QFont (Constants::Settings::DEFAULT_FONT_FAMILY,
               Constants::Settings::TITLE_FONT_SIZE));

    homePageLayout->addWidget (randomRecommendationLabel);

    // Random Recommendation Layout
    randomRecommendationLayout = new QHBoxLayout (centralWidget);
    randomRecommendationLayout->setAlignment (Qt::AlignHCenter);
    randomRecommendationLayout->setSpacing (20);

    recommendWordButton =
        new ElaPushButton (tr ("Recommend Word"), centralWidget);
    recommendWordButton->setMinimumHeight (50);
    recommendWordButton->setMinimumWidth (600);
    recommendWordButton->setBorderRadius (8);
    recommendWordButton->setToolTip (tr ("Get a random word recommendation"));

    randomRecommendationLayout->addWidget (recommendWordButton);

    homePageLayout->addLayout (randomRecommendationLayout);

    addCentralWidget (centralWidget, true, true, 0);
}

void HomePage::initConnections ()
{
    connect (swapButton, &ElaIconButton::clicked,
             [=, this] ()
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
             [=, this] ()
             {
                 if (searchOnline->getIsToggled ())
                 {
                     QDesktopServices::openUrl (QUrl (
                         Constants::Urls::BING_SEARCH.arg (lineEdit->text ())));
                     return;
                 }
             });

    connect (clearAction, &QAction::triggered,
             [=, this] ()
             {
                 lineEdit->clear ();

                 suggestionModel->setStringList (QStringList ());
                 suggestionsList->hide ();
             });

    connect (searchMode_precise, &ElaRadioButton::toggled,
             [=, this] (bool checked)
             {
                 if (checked && !lineEdit->text ().isEmpty ())
                 {
                     // Clear current suggestions and trigger new search
                     suggestionModel->setStringList (QStringList ());
                     // Trigger the same logic as textEdited
                     emit lineEdit->textEdited (lineEdit->text ());
                 }
             });

    connect (searchMode_fuzzy, &ElaRadioButton::toggled,
             [=, this] (bool checked)
             {
                 if (checked && !lineEdit->text ().isEmpty ())
                 {
                     // Clear current suggestions and trigger new search
                     suggestionModel->setStringList (QStringList ());
                     // Trigger the same logic as textEdited
                     emit lineEdit->textEdited (lineEdit->text ());
                 }
             });

    connect (
        lineEdit, &ElaLineEdit::textEdited,
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

                if (LangComboBox_left->currentText () == tr ("English") &&
                    LangComboBox_right->currentText () == tr ("Chinese"))
                {
                    if (searchMode_precise->isChecked ())
                    {
                        suggestionModel->setStringList (QStringList ());
                        auto wordEntry = DbManager::getInstance ().lookupWord (
                            lineEdit->text (),
                            mapLang (LangComboBox_left->currentText ()));
                        if (wordEntry.has_value ())
                        {
                            QStringList singleSuggestion;
                            singleSuggestion << wordEntry->word;
                            suggestionModel->setStringList (singleSuggestion);
                            suggestionsList->setModel (suggestionModel);
                        }
                        else
                        {
                            suggestionModel->setStringList (QStringList ());
                            suggestionsList->hide ();
                        }
                    }

                    if (searchMode_fuzzy->isChecked ())
                    {

                        suggestionModel->setStringList (QStringList ());
                        auto wordEntries =
                            DbManager::getInstance ().searchWords (
                                lineEdit->text (),
                                mapLang (LangComboBox_left->currentText ()));
                        QStringList suggestions;
                        for (const auto &entry : wordEntries)
                        {
                            suggestions << entry.word;
                        }
                        suggestionModel->setStringList (suggestions);
                        suggestionsList->setModel (suggestionModel);
                    }
                }
                else if (LangComboBox_left->currentText () == tr ("Chinese") &&
                         LangComboBox_right->currentText () == tr ("English"))
                {
                    if (searchMode_precise->isChecked ())
                    {
                        suggestionModel->setStringList (QStringList ());
                        auto wordEntry = DbManager::getInstance ().lookupWord (
                            lineEdit->text (),
                            mapLang (LangComboBox_left->currentText ()));
                        if (wordEntry.has_value ())
                        {
                            QStringList singleSuggestion;
                            singleSuggestion << wordEntry->word;
                            suggestionModel->setStringList (singleSuggestion);
                            suggestionsList->setModel (suggestionModel);
                        }
                        else
                        {
                            suggestionModel->setStringList (QStringList ());
                            suggestionsList->hide ();
                        }
                    }

                    if (searchMode_fuzzy->isChecked ())
                    {
                        suggestionModel->setStringList (QStringList ());
                        auto wordEntries =
                            DbManager::getInstance ().searchWords (
                                lineEdit->text (),
                                mapLang (LangComboBox_left->currentText ()),
                                20);
                        QStringList suggestions;
                        for (const auto &entry : wordEntries)
                        {
                            suggestions << entry.word;
                        }
                        suggestionModel->setStringList (suggestions);
                        suggestionsList->setModel (suggestionModel);
                    }
                }
                else
                {
                    suggestionsList->hide ();
                    return;
                }
                suggestionsList->show ();
            }
        });

    connect (lineEdit, &ElaLineEdit::returnPressed,
             [=, this] ()
             {
                 if (searchOnline->getIsToggled ())
                 {
                     if (lineEdit->text ().isEmpty ())
                     {
                         return;
                     }
                     QDesktopServices::openUrl (QUrl (
                         Constants::Urls::BING_SEARCH.arg (lineEdit->text ())));
                     return;
                 }
                 else
                 {
                     if (searchMode_precise->isChecked ())
                     {
                         auto wordEntry = DbManager::getInstance ().lookupWord (
                             lineEdit->text (),
                             mapLang (LangComboBox_left->currentText ()));

                         if (wordEntry.has_value ())
                         {
                             auto wordCard = WordCard::getInstance ();
                             wordCard->setWordEntry (wordEntry.value ());
                             wordCard->show ();
                         }
                         else
                         {
                             showDialog (tr ("Error Loading Word Card"),
                                         tr ("The word you entered can't be "
                                             "loaded from the database. "));
                         }
                     }

                     if (searchMode_fuzzy->isChecked ())
                     {
                         auto firstWordEntry =
                             DbManager::getInstance ().searchWords (
                                 lineEdit->text (),
                                 mapLang (LangComboBox_left->currentText ()),
                                 1);

                         if (!firstWordEntry.empty ())
                         {
                             auto wordCard = WordCard::getInstance ();
                             wordCard->setWordEntry (firstWordEntry[0]);
                             wordCard->show ();
                         }
                         else
                         {
                             showDialog (tr ("Error Loading Word Card"),
                                         tr ("The word you entered can't be "
                                             "loaded from the database. "));
                         }
                     }
                 }
             });

    connect (suggestionsList, &ElaListView::clicked, this,
             [=, this] (const QModelIndex &index)
             {
                 if (searchOnline->getIsToggled ())
                 {
                     QDesktopServices::openUrl (
                         QUrl (Constants::Urls::BING_SEARCH.arg (
                             index.data ().toString ())));
                     return;
                 }
                 auto wordEntry = DbManager::getInstance ().lookupWord (
                     index.data ().toString (), "en");
                 if (!wordEntry.has_value ())
                 {
                     showDialog (
                         tr ("Error Loading Word Card"),
                         tr ("The word you selected can't be loaded from "
                             "the database. "));
                 }
                 else
                 {
                     auto wordCard = WordCard::getInstance ();

                     wordCard->setWordEntry (wordEntry.value ());

                     wordCard->show ();
                 }
             });

    connect (recommendWordButton, &ElaPushButton::clicked,
             [=] ()
             {
                 auto wordEntry = DbManager::getInstance ().getRandomWord ();
                 if (wordEntry.has_value ())
                 {
                     auto wordCard = WordCard::getInstance ();
                     wordCard->setWordEntry (wordEntry.value ());
                     wordCard->show ();
                 }
             });
}