#pragma once
#include "Utility/ClickableWidget.h"
#include "Utility/WordEntry.h"
#include "View/TempPage.h"
#include <utility>

class QuizCard;

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

        emit progressUpdated (currentProgress, totalProgress);
    }

signals:
    void progressUpdated (int current, int total);

private slots:
    void onLoginSuccessful ();
    void onLogoutSuccessful ();
    void onReciteButtonClicked ();

    void updateProgressUI (int current, int total);

    void handleQuizCardOptionSelected ();

private:
    void initUI ();
    void initConnections ();
    void initializeCardAmount ();
    void showNextQuizCard ();
    void showCompletionDialog ();

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

    QuizCard *quizCard;

    int currentProgress = 0;
    int totalProgress = 15;

    std::vector<WordEntry> Card_amount;
    int currentCardIndex = 0;
};