#pragma once

#include "Utility/Constants.h"
#include "Utility/WordEntry.h"
#include "View/TempPage.h"

class WordCard : public TempPage
{
    Q_OBJECT

public:
    static WordCard *getInstance (QWidget *parent = nullptr)
    {
        static WordCard instance (parent);
        return &instance;
    }

    void setAdd2FavoritesButtonEnabled (bool enabled);

    void setWordEntry (WordEntry &entry);

    void addToUserFavorites (const QString &userId, const QString &word);

    void removeFromUserFavorites (const QString &userId, const QString &word);
private slots:
    void onAdd2FavoritesClicked ();

private:
    explicit WordCard (QWidget *parent = nullptr);
    WordCard () = delete;
    WordCard (const WordCard &) = delete;
    WordCard &operator= (const WordCard &) = delete;

    void initUI ();
    void initConnections ();

    bool isFavorite () const;

    void updateAdd2FavoritesButton (); // update icon and tooltip

    // UI components
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;
    QHBoxLayout *headerLayout;
    QLabel *wordLabel;
    QLabel *pronunciationLabel;
    QFrame *separatorLine;
    QLabel *translationLabel;

    ElaIconButton *add2FavoritesButton =
        new ElaIconButton (Constants::UI::UNLIKE_ICON); // Default unfavortite

    WordEntry currentWordEntry;
};
