#pragma once
#include "View/TempPage.h"

class StatisticsPage : public TempPage
{
    Q_OBJECT

public:
    explicit StatisticsPage (QWidget *parent = nullptr);

private:
    void initUI ();
    void initConnections ();

    // UI components
    QWidget *centralWidget;
    QVBoxLayout *statisticsPageLayout;
    QWidget *statsWidget;
    QVBoxLayout *statsLayout;
    QLabel *titileLabel;
    QLabel *masteredWordsLabel;
    QLabel *learningWordsLabel;
    ElaIconButton *refreshButton;
    QWidget *masteredWordsWidget;
    QVBoxLayout *masteredWordsLayout;
    QLabel *masteredWordsListLabel;
    ElaScrollArea *masteredWordsScrollArea;
    ElaListView *masteredWordsList;
};