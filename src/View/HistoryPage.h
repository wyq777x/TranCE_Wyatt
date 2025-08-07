#pragma once
#include "View/TempPage.h"

class HistoryPage : public TempPage
{
    Q_OBJECT
public:
    explicit HistoryPage (QWidget *parent = nullptr);

private slots:
    void enableHistorySearchListUI (bool enabled);

private:
    void initUI ();
    void initConnections ();
    void loadSearchHistory ();
    void loadReciteHistory ();

    // UI components
    QWidget *centralWidget;
    QVBoxLayout *layout;

    QStringListModel *historySearchModel;
    QStringListModel *reciteHistoryModel;
    ElaListView *searchHistoryListView;
    ElaListView *reciteHistoryListView;

    QLabel *searchHistoryLabel;
    QLabel *reciteHistoryLabel;
};