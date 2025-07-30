#pragma once

#include "Utility/WordEntry.h"
#include <QObject>

class UIController : public QObject
{

    Q_OBJECT
public:
    static UIController &getInstance ()
    {
        static UIController instance;
        return instance;
    }

    void showWordCard (WordEntry &entry, QWidget *parent = nullptr);
    void notifySearchHistoryUpdated ();

signals:
    void historySearchListUIChanged (bool enabled);
    void searchHistoryUpdated ();

private slots:
    void enableHistorySearchListUI (bool enabled);

private:
    explicit UIController () { initConnections (); }

    UIController (const UIController &) = delete;
    UIController &operator= (const UIController &) = delete;
    UIController (UIController &&) = delete;
    UIController &operator= (UIController &&) = delete;

    void initConnections ();
};