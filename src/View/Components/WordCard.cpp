#include "WordCard.h"
#include "Utility/Constants.h"

WordCard::WordCard (QWidget *parent) : TempPage (parent)
{
    setWindowTitle (tr ("Word Card"));
    setFixedSize (400, 400);
    setStyleSheet (
        "background-color: #f0f0f0; font-family: 'Noto Sans', sans-serif;");

    initUI ();
    initConnections ();
}

void WordCard::initUI ()
{
    // UI Layout:
    //   BIG WORD     ADD2FAVORITES_BUTTON
    //   SMALL PRONUNCIATION
    //   -------------------
    //   TRANSLATION

    centralWidget = new QWidget (this);
    centralWidget->setWindowTitle (tr ("Word Card"));

    mainLayout = new QVBoxLayout (centralWidget);
    mainLayout->setContentsMargins (20, 20, 20, 20);
    mainLayout->setSpacing (10);

    headerLayout = new QHBoxLayout ();

    wordLabel = new QLabel ();
    wordLabel->setAlignment (Qt::AlignCenter);
    wordLabel->setStyleSheet (QString ("QLabel {"
                                       "font-size: %1px;"
                                       "font-weight: bold;"
                                       "color: #2c3e50;"
                                       "padding: 10px;"
                                       "}")
                                  .arg (Constants::Settings::TITLE_FONT_SIZE));

    headerLayout->addStretch ();
    headerLayout->addWidget (wordLabel);
    headerLayout->addStretch ();

    add2FavoritesButton->setFixedSize (Constants::UI::ICON_SIZE,
                                       Constants::UI::ICON_SIZE);
    headerLayout->addWidget (add2FavoritesButton);

    pronunciationLabel = new QLabel ();
    pronunciationLabel->setAlignment (Qt::AlignCenter);
    pronunciationLabel->setStyleSheet (
        QString ("QLabel {"
                 "font-size: %1px;"
                 "color: #7f8c8d;"
                 "font-style: italic;"
                 "padding: 5px;"
                 "}")
            .arg (Constants::Settings::SMALL_FONT_SIZE));

    separatorLine = new QFrame ();
    separatorLine->setFrameShape (QFrame::HLine);
    separatorLine->setFrameShadow (QFrame::Sunken);
    separatorLine->setStyleSheet ("QFrame {"
                                  "color: #bdc3c7;"
                                  "margin: 10px 0px;"
                                  "}");

    translationLabel = new QLabel ();
    translationLabel->setAlignment (Qt::AlignLeft | Qt::AlignTop);
    translationLabel->setMinimumHeight (100);
    translationLabel->setWordWrap (true);
    translationLabel->setStyleSheet (
        QString ("QLabel {"
                 "font-size: %1px;"
                 "color: #34495e;"
                 "padding: 15px;"
                 "background-color: #ffffff;"
                 "border: 1px solid #ecf0f1;"
                 "border-radius: 8px;"
                 "}")
            .arg (Constants::Settings::DEFAULT_FONT_SIZE));

    mainLayout->addLayout (headerLayout);
    mainLayout->addWidget (pronunciationLabel);
    mainLayout->addWidget (separatorLine);
    mainLayout->addWidget (translationLabel);
    mainLayout->addStretch ();

    addCentralWidget (centralWidget);
}

void WordCard::initConnections ()
{
    connect (add2FavoritesButton, &ElaIconButton::clicked, this,
             [this] ()
             {
                 // Building...

                 add2FavoritesButton->setAwesome (Constants::UI::LIKE_ICON);
                 qDebug () << "Add to Favorites clicked";
             });
}

void WordCard::setAdd2FavoritesButtonEnabled (bool enabled)
{

    add2FavoritesButton->setEnabled (enabled);
    add2FavoritesButton->setVisible (enabled);
}

void WordCard::setWordEntry (WordEntry &entry)
{

    wordLabel->setText (entry.word);

    if (!entry.pronunciation.isEmpty ())
    {
        pronunciationLabel->setText (
            QString ("/%1/").arg (entry.pronunciation));
    }
    else
    {
        pronunciationLabel->setText ("");
    }

    QString translationText = entry.translation;
    translationText.replace ("\\n", "\n");
    translationText.replace ("\\t", "\t");
    translationText.replace ("\\r", "\r");

    if (!entry.partOfSpeech.isEmpty ())
    {

        translationText =
            QString ("[%1] %2").arg (entry.partOfSpeech, translationText);
    }

    if (!entry.examples.isEmpty ())
    {
        QString examples = entry.examples;
        examples.replace ("\\n", "\n");

        translationText += QString ("\n\nExamples: %1").arg (examples);

        if (!entry.exampleTranslation.isEmpty ())
        {
            QString exampleTranslation = entry.exampleTranslation;
            exampleTranslation.replace ("\\n", "\n");
            translationText +=
                QString ("\nTranslation: %1").arg (exampleTranslation);
        }
    }

    translationLabel->setText (translationText);
}
