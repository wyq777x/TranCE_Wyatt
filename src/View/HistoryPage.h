#pragma once
#include "View/TempPage.h"

class HistoryPage : public TempPage
{
    Q_OBJECT
public:
    explicit HistoryPage (QWidget *parent = nullptr);

private:
    void initUI ();
    void initConnections ();

    // UI components
    QWidget *centralWidget;
    QVBoxLayout *layout;
    ElaListView *searchHistoryListView;
    ElaListView *reciteHistoryListView;
};