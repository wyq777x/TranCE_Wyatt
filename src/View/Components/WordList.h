#pragma once

#include "View/TempPage.h"

class WordList : public TempPage
{
    Q_OBJECT

public:
    static WordList *getInstance (QWidget *parent = nullptr)
    {
        static WordList instance (parent);
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
