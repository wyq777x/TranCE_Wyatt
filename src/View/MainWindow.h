#pragma once

#include <ElaTheme.h>
#include <ElaWindow.h>

class AboutPage;
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
    void initPages ();
    ~MainWindow () {};

private:
    AboutPage *aboutPage;
    HistoryPage *historyPage;
    HomePage *homePage;
    LoginPage *loginPage;
    MyPage *myPage;
    RecitePage *recitePage;
    SettingPage *settingPage;
    StatisticsPage *statisticsPage;
private slots:
    void onLoginSuccessful (const QString &username);
    void onLogoutSuccessful ();
};