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

    void setAdd2FavoritesButtonEnabled (bool enabled);

    void setWordEntry (WordEntry &entry);

    void fillReciteOptions ();

    void shuffleReciteOptions ();

    void setReciteOptions (const std::vector<QString> &options);

    std::vector<QString> getReciteOptions () const { return reciteOptions; }

    void addToUserFavorites (const QString &userId, const QString &word);

    void removeFromUserFavorites (const QString &userId, const QString &word);

    bool validateAnswer (int optionIndex) const;
    void showAnswerFeedback (int selectedOptionIndex, bool isCorrect);
    void resetButtonStates ();

signals:
    void optionSelected (int optionIndex, bool isCorrect);

    void masterButtonClicked ();

private slots:
    void onAdd2FavoritesClicked ();
    void onMasterButtonClicked ();

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

    QPushButton *optionAButton;
    QPushButton *optionBButton;
    QPushButton *optionCButton;
    QPushButton *optionDButton;

    ElaIconButton *add2FavoritesButton =
        new ElaIconButton (Constants::UI::UNLIKE_ICON);
    ElaIconButton *masterButton = new ElaIconButton (ElaIconType::Book);

    WordEntry currentWordEntry;

    std::vector<WordEntry> reciteWordEntries;
    int currentReciteIndex = 0;

    std::vector<QString> reciteOptions;

    int correctAnswerIndex = 0; // Index of the correct answer in reciteOptions
};