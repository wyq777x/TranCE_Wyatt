#pragma once

#include "View/TempPage.h"

class QuizCard : public TempPage

{
    Q_OBJECT
public:
    static QuizCard *getInstance (QWidget *parent = nullptr)
    {
        static QuizCard instance (parent);
        return &instance;
    }

private:
    explicit QuizCard (QWidget *parent = nullptr);

    QuizCard () = delete;
    QuizCard (const QuizCard &) = delete;
    QuizCard &operator= (const QuizCard &) = delete;

    void initUI ();
    void initConnections ();

    // UI components
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;
    QHBoxLayout *headerLayout;
    QLabel *wordLabel;
    QFrame *separatorLine;

    ElaPushButton *optionAButton;
    ElaPushButton *optionBButton;
    ElaPushButton *optionCButton;
    ElaPushButton *optionDButton;

    ElaIconButton *add2FavoritesButton = new ElaIconButton (ElaIconType::Heart);
    ElaIconButton *masterButton = new ElaIconButton (ElaIconType::Book);
};