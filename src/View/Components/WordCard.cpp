#include "Components/WordCard.h"

WordCard::WordCard (QWidget *parent) : TempPage (parent)
{
    // UI Layout:
    //   BIG WORD
    //   SMALL PRONUNCIATION
    //   -------------------
    //   TRANSLATION

    setWindowTitle ("Word Card");

    setStyleSheet (
        "background-color: #f0f0f0; font-family: 'Noto Sans', sans-serif;");

    QVBoxLayout *mainLayout = new QVBoxLayout ();
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
    translationLabel->setWordWrap (true);
    translationLabel->setMinimumHeight (100);
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
    if (!entry.partOfSpeech.isEmpty ())
    {
        translationText =
            QString ("[%1] %2").arg (entry.partOfSpeech, entry.translation);
    }

    if (!entry.examples.isEmpty ())
    {
        translationText += QString ("\n\nExamples: %1").arg (entry.examples);
        if (!entry.exampleTranslation.isEmpty ())
        {
            translationText +=
                QString ("\nTranslation: %1").arg (entry.exampleTranslation);
        }
    }

    translationLabel->setText (translationText);
}