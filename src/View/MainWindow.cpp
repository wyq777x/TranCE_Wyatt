#include "MainWindow.h"
MainWindow::MainWindow (QWidget *parent) : ElaWindow (parent)
{
    setWindowTitle ("TranCE_Wyatt");
    setWindowIcon (QIcon (":image/learnENG.ico"));

    initPages ();
    moveToCenter ();

    setUserInfoCardTitle ("User");
    setUserInfoCardSubTitle ("Click to login");
    setUserInfoCardPixmap (QPixmap (":image/DefaultUser.png"));

    connect (this, &ElaWindow::userInfoCardClicked, this,
             [this] ()
             {
                 if (!AccountManager::getInstance ().isLoggedIn ())
                 {
                     // If not logged in, show login page
                     loginPage->show ();
                 }
                 else
                 {
                     myPage->show ();
                 }
             });

    connect (&AccountManager::getInstance (), &AccountManager::loginSuccessful,
             this, &MainWindow::onLoginSuccessful);
}

void MainWindow::initPages ()
{
    // PageNodes
    homePage = new HomePage ();
    addPageNode ("Home", homePage, ElaIconType::House);

    recitePage = new RecitePage ();
    addPageNode ("Recite", recitePage, ElaIconType::Book);

    historyPage = new HistoryPage ();
    addPageNode ("History", historyPage, ElaIconType::ClockRotateLeft);

    statisticsPage = new StatisticsPage ();
    addPageNode ("Statistics", statisticsPage, ElaIconType::ChartBar);

    // FooterNodes

    settingPage = new SettingPage ();
    QString settingPageKey;
    addFooterNode ("Settings", settingPage, settingPageKey, 0,
                   ElaIconType::Gear);

    aboutPage = new AboutPage ();
    QString aboutPageKey;
    addFooterNode ("About", aboutPage, aboutPageKey, 0,
                   ElaIconType::CircleInfo);

    loginPage = new LoginPage ();
    loginPage->setStyleSheet ("background-color:rgb(231, 252, 249);"
                              "QWidget { background-color: transparent; }");

    myPage = new MyPage ();
}

void MainWindow::onLoginSuccessful (const QString &username)
{
    setUserInfoCardTitle (username);
    setUserInfoCardSubTitle ("Click to open MyPage");

    myPage->usernameTextLabel->setText (
        QString ("Username: %1").arg (username));

    // setUserInfoCardPixmap (QPixmap (const QString &avatarPath));

    // to be further designed
}