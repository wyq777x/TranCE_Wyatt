#pragma once

#include "Utility/Constants.h"
#include "Utility/WordEntry.h"
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

    void setWordEntry (WordEntry &entry);

    // extract 3 translations randomly and push them into reciteOptions

    void fillReciteOptions ();

    // shuffle reciteOptions

    void shuffleReciteOptions ();

    // set reciteOptions in quizCard

    void setReciteOptions (const std::vector<QString> &options);

private:
    explicit QuizCard (QWidget *parent = nullptr);

    QuizCard () = delete;
    QuizCard (const QuizCard &) = delete;
    QuizCard &operator= (const QuizCard &) = delete;

    void initUI ();
    void initConnections ();

    bool isFavorite () const;

    void updateAdd2FavoritesButton (); // update icon and tooltip

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

    ElaIconButton *add2FavoritesButton =
        new ElaIconButton (Constants::UI::UNLIKE_ICON);
    ElaIconButton *masterButton = new ElaIconButton (ElaIconType::Book);

    WordEntry currentWordEntry;

    std::vector<WordEntry> reciteWordEntries;
    int currentReciteIndex = 0;

    std::vector<QString> reciteOptions;
};