#pragma once
#include "Utility/ClickableWidget.h"
#include "View/TempPage.h"
#include <utility>

class RecitePage : public TempPage
{
    Q_OBJECT
public:
    explicit RecitePage (QWidget *parent = nullptr);

    std::pair<int, int> getProgress () const
    {
        return {currentProgress, totalProgress};
    }
    void setProgress (int current, int total)
    {
        currentProgress = current;
        totalProgress = total;
    }

private slots:
    void onReciteButtonClicked ();

private:
    void initUI ();
    void initConnections ();

    // UI components
    QWidget *centralWidget;
    QVBoxLayout *recitePageLayout;
    QWidget *containerWidget;
    QGridLayout *containerLayout;
    QWidget *reciteBgWidget;
    QWidget *reciteContentWidget;
    QVBoxLayout *reciteWidgetLayout;
    QLabel *progressLabel;
    ElaPushButton *reciteButton;
    QWidget *HBoxContainer;
    QHBoxLayout *HBoxLayout;
    ClickableWidget *favoritesWidget;
    QWidget *favoritesContentWidget;
    QLabel *favoritesLabel;
    QVBoxLayout *favoritesLayout;
    QHBoxLayout *favoritesWidgetLayout;
    ClickableWidget *masteredWidget;
    QWidget *masteredContentWidget;
    QLabel *masteredLabel;
    QVBoxLayout *masteredLayout;
    QHBoxLayout *masteredWidgetLayout;

    int currentProgress = 0;
    int totalProgress = 50;
};