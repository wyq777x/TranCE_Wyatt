#pragma once
#include "ElaWindow.h"
#include "View/TempPage.h"
#include "View/StatisticsPage.h"
#include "View/MyPage.h"
#include "View/RecitePage.h"
#include "View/AboutPage.h"
#include "View/HistoryPage.h"
#include "View/LoginPage.h"
#include "View/HomePage.h"
#include "View/SettingPage.h"
class MainWindow : public ElaWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    void initPages();
    void initModels();
    ~MainWindow() {};

private:
    AboutPage *aboutPage;
    HistoryPage *historyPage;
    HomePage *homePage;
    LoginPage *loginPage;
    MyPage *myPage;
    RecitePage *recitePage;
    SettingPage *settingPage;
    StatisticsPage *statisticsPage;
};