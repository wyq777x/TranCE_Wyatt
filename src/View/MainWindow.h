#pragma once

#include <ElaWindow.h>

class AboutPage;
class AIPage;
class HistoryPage;
class HomePage;
class LoginPage;
class MyPage;
class RecitePage;
class SettingPage;
class StatisticsPage;

class MainWindow : public ElaWindow
{
    Q_OBJECT

public:
    explicit MainWindow (QWidget *parent = nullptr);
    ~MainWindow () {};

    // Opens the AI mode page (navigation + sidecar startup). Returns false
    // when AI mode is disabled or not configured for the current profile.
    bool openAiMode ();

private slots:
    void onLoginSuccessful (const QString &username);
    void onLogoutSuccessful ();

private:
    void initPages ();
    void initConnections ();

    // Navigation key of the AI footer node, needed for programmatic
    // navigation (Ela generates random node keys).
    QString aiPageKey;

    // UI components/pages
    AboutPage *aboutPage;
    AIPage *aiPage;
    HistoryPage *historyPage;
    HomePage *homePage;
    LoginPage *loginPage;
    MyPage *myPage;
    RecitePage *recitePage;
    SettingPage *settingPage;
    StatisticsPage *statisticsPage;
};