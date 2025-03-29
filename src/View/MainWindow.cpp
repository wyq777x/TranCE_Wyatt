#include "MainWindow.h"
MainWindow::MainWindow(QWidget *parent)
    : ElaWindow(parent)
{
    setWindowTitle("TranCE_Wyatt");
    setWindowIcon(QIcon(":res/image/learnENG.ico"));

    initPages();
    initModels();
    moveToCenter();

    setUserInfoCardTitle("User");
    setUserInfoCardSubTitle("Click to login");
    setUserInfoCardPixmap(QPixmap(":/res/image/DefaultUser.png"));
}

void MainWindow::initPages()
{
    homePage = new HomePage();
    addPageNode("Home", homePage, ElaIconType::House);
    recitePage = new RecitePage();
    addPageNode("Recite", recitePage, ElaIconType::Book);
    historyPage = new HistoryPage();
    addPageNode("History", historyPage, ElaIconType::ClockRotateLeft);
    statisticsPage = new StatisticsPage();
    addPageNode("Statistics", statisticsPage, ElaIconType::ChartBar);
    aboutPage = new AboutPage();
    QString aboutPageKey;
    addFooterNode("About", aboutPage, aboutPageKey, 0, ElaIconType::CircleInfo);

    // to be used
    myPage = new MyPage();
    loginPage = new LoginPage();
}

void MainWindow::initModels()
{
    // Initialize models here
}