#pragma once
#include "View/TempPage.h"

class ElaIconButton;
class QLabel;
class QVBoxLayout;
class QWidget;

class StatisticsPage : public TempPage
{
    Q_OBJECT

public:
    explicit StatisticsPage (QWidget *parent = nullptr);

private slots:
    void onRefreshButtonClicked ();

private:
    void initUI ();
    void initConnections ();
    void refreshStatistics ();

    // UI components
    QWidget *centralWidget;
    QVBoxLayout *statisticsPageLayout;
    QWidget *statsWidget;
    QVBoxLayout *statsLayout;
    QLabel *titileLabel;
    QLabel *masteredWordsLabel;
    QLabel *learningWordsLabel;
    ElaIconButton *refreshButton;
};