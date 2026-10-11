#pragma once

#include "Utility/Constants.h"
#include "Utility/WordEntry.h"
#include "View/TempPage.h"
#include <ElaIconButton.h>

class QFrame;
class QHBoxLayout;
class QLabel;
class QVBoxLayout;
class QWidget;

class WordCard : public TempPage
{
    Q_OBJECT

public:
    static WordCard *getInstance (QWidget *parent = nullptr)
    {
        // Process-wide singleton: it must never be owned by a transient
        // parent, or the first caller's widget destruction would delete it
        // and leave this static pointer dangling. The parameter is kept for
        // call-site compatibility but deliberately ignored.
        Q_UNUSED (parent);
        static WordCard instance (nullptr);
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
