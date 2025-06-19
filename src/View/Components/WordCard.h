#pragma once

#include "Utility/WordEntry.h"
#include "View/TempPage.h"
#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>
#include <qevent.h>

class WordCard : public TempPage
{
    Q_OBJECT

public:
    static WordCard *getInstance (QWidget *parent = nullptr)
    {
        static WordCard instance (parent);
        return &instance;
    }

    void setWordEntry (WordEntry &entry);

private:
    explicit WordCard (QWidget *parent = nullptr);
    WordCard () = delete;
    WordCard (const WordCard &) = delete;
    WordCard &operator= (const WordCard &) = delete;

    // UI components
    QLabel *wordLabel;
    QLabel *pronunciationLabel;
    QFrame *separatorLine;
    QLabel *translationLabel;
};
