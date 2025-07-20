#include "Components/WordCard.h"
#include "Constants.h"
#include "Def.h"
#include "ElaIconButton.h"
#include <qboxlayout.h>
#include <qnamespace.h>

WordCard::WordCard (QWidget *parent) : TempPage (parent)
{
    // UI Layout:
    //   BIG WORD     ADD2FAVORITES_BUTTON
    //   SMALL PRONUNCIATION
    //   -------------------
    //   TRANSLATION

    setWindowTitle (tr ("Word Card"));

    setFixedSize (400, 400);
    setStyleSheet (
        "background-color: #f0f0f0; font-family: 'Noto Sans', sans-serif;");

    QVBoxLayout *mainLayout = new QVBoxLayout (this);
    mainLayout->setContentsMargins (20, 20, 20, 20);
    mainLayout->setSpacing (10);

    QHBoxLayout *headerLayout = new QHBoxLayout ();

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

    QWidget *container = new QWidget ();
    container->setLayout (mainLayout);
    container->setWindowTitle (tr ("Word Card"));
    addCentralWidget (container);

    connect (add2FavoritesButton, &ElaIconButton::clicked, this,
             [this] ()
             {
                 // Building...

                 add2FavoritesButton->setAwesome (
                     ElaIconType::HeartCircleCheck);
                 qDebug () << "Add to Favorites clicked";
             });
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
