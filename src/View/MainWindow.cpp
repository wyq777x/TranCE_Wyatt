#include "MainWindow.h"
#include "AccountManager.h"
MainWindow::MainWindow (QWidget *parent) : ElaWindow (parent)
{
    setWindowTitle ("TranCE_Wyatt");
    setWindowIcon (QIcon (":image/learnENG.ico"));

    initPages ();
    moveToCenter ();

    setUserInfoCardTitle ("User");
    setUserInfoCardSubTitle (
        QString ("%1").arg (AccountManager::getInstance ().isLoggedIn ()
                                ? AccountManager::getInstance ().getEmail ()
                                : "Click to login"));

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

    connect (myPage, &MyPage::usernameChanged, this,
             [this] (const QString &newUsername)
             { setUserInfoCardTitle (newUsername); });

    connect (myPage, &MyPage::emailChanged, this,
             [this] (const QString &newEmail)
             { setUserInfoCardSubTitle (newEmail); });
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

    setUserInfoCardSubTitle (
        QString ("%1").arg (AccountManager::getInstance ().getEmail ()));

    myPage->usernameLineEdit->setText (username);

    myPage->emailLineEdit->setText (AccountManager::getInstance ().getEmail ());

    // setUserInfoCardPixmap (QPixmap (const QString &avatarPath));

    // to be further designed
}