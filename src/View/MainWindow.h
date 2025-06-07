#pragma once
#include "Model/UserModel.h"
#include "View/AboutPage.h"
#include "View/HistoryPage.h"
#include "View/HomePage.h"
#include "View/LoginPage.h"
#include "View/MyPage.h"
#include "View/RecitePage.h"
#include "View/SettingPage.h"
#include "View/StatisticsPage.h"
#include "View/TempPage.h"
#include <ElaTheme.h>
#include <ElaWindow.h>

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
};