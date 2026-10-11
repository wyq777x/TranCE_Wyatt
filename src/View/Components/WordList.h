#pragma once

#include "View/TempPage.h"
#include <QString>
#include <QStringList>

class ElaListView;
class QModelIndex;
class QStringListModel;
class QVBoxLayout;
class QWidget;

class WordList : public TempPage
{
    Q_OBJECT

public:
    static WordList *getInstance (QWidget *parent = nullptr)
    {
        // Process-wide singleton: it must never be owned by a transient
        // parent, or the first caller's widget destruction would delete it
        // and leave this static pointer dangling. The parameter is kept for
        // call-site compatibility but deliberately ignored.
        Q_UNUSED (parent);
        static WordList instance (nullptr);
        return &instance;
    }

    void setTitle (const QString &title);
    void setWordList (const QStringList &words);

private slots:
    void onWordClicked (const QModelIndex &index);

private:
    explicit WordList (QWidget *parent = nullptr);
    WordList () = delete;
    WordList (const WordList &) = delete;
    WordList &operator= (const WordList &) = delete;

    void initUI ();
    void initConnections ();

    // UI components
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;
    ElaListView *wordListView;
    QStringListModel *wordListModel;

    QString currentTitle;
    QStringList currentWords;
};
