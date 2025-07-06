#include "Components/WordCard.h"
#include <qnamespace.h>

WordCard::WordCard (QWidget *parent) : TempPage (parent)
{
    // UI Layout:
    //   BIG WORD
    //   SMALL PRONUNCIATION
    //   -------------------
    //   TRANSLATION

    setWindowTitle ("Word Card");

    setFixedSize (400, 400);
    setStyleSheet (
        "background-color: #f0f0f0; font-family: 'Noto Sans', sans-serif;");

    QVBoxLayout *mainLayout = new QVBoxLayout (this);
    mainLayout->setContentsMargins (20, 20, 20, 20);
    mainLayout->setSpacing (10);

    wordLabel = new QLabel ();
    wordLabel->setAlignment (Qt::AlignCenter);
    wordLabel->setStyleSheet ("QLabel {"
                              "font-size: 24px;"
                              "font-weight: bold;"
                              "color: #2c3e50;"
                              "padding: 10px;"
                              "}");

    pronunciationLabel = new QLabel ();
    pronunciationLabel->setAlignment (Qt::AlignCenter);
    pronunciationLabel->setStyleSheet ("QLabel {"
                                       "font-size: 14px;"
                                       "color: #7f8c8d;"
                                       "font-style: italic;"
                                       "padding: 5px;"
                                       "}");

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
    translationLabel->setStyleSheet ("QLabel {"
                                     "font-size: 16px;"
                                     "color: #34495e;"
                                     "padding: 15px;"
                                     "background-color: #ffffff;"
                                     "border: 1px solid #ecf0f1;"
                                     "border-radius: 8px;"
                                     "}");

    mainLayout->addWidget (wordLabel);
    mainLayout->addWidget (pronunciationLabel);
    mainLayout->addWidget (separatorLine);
    mainLayout->addWidget (translationLabel);
    mainLayout->addStretch ();

    QWidget *container = new QWidget ();
    container->setLayout (mainLayout);
    container->setWindowTitle ("Word Card");
    addCentralWidget (container);
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