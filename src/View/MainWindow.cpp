#include "MainWindow.h"
#include "Controller/AccountManager.h"
#include "Utility/Constants.h"
#include "View/AboutPage.h"
#include "View/Components/LoginPage.h"
#include "View/Components/MyPage.h"
#include "View/HistoryPage.h"
#include "View/HomePage.h"
#include "View/RecitePage.h"
#include "View/SettingPage.h"
#include "View/StatisticsPage.h"

MainWindow::MainWindow (QWidget *parent) : ElaWindow (parent)
{
    setWindowTitle (tr ("TranCE_Wyatt"));
    setWindowIcon (QIcon (Constants::Resources::APP_ICON));

    initPages ();
    initConnections ();
    moveToCenter ();

    setUserInfoCardTitle (tr ("User"));
    setUserInfoCardSubTitle (
        QString ("%1").arg (AccountManager::getInstance ().isLoggedIn ()
                                ? AccountManager::getInstance ().getEmail ()
                                : tr ("Click to login")));

    setUserInfoCardPixmap (QPixmap (Constants::Resources::DEFAULT_USER_AVATAR));
}

void MainWindow::initPages ()
{
    // PageNodes
    homePage = new HomePage ();
    addPageNode (tr ("Home"), homePage, ElaIconType::House);

    recitePage = new RecitePage ();
    addPageNode (tr ("Recite"), recitePage, ElaIconType::Book);

    historyPage = new HistoryPage ();
    addPageNode (tr ("History"), historyPage, ElaIconType::ClockRotateLeft);

    statisticsPage = new StatisticsPage ();
    addPageNode (tr ("Statistics"), statisticsPage, ElaIconType::ChartBar);

    // FooterNodes

    settingPage = new SettingPage ();
    QString settingPageKey;
    addFooterNode (tr ("Settings"), settingPage, settingPageKey, 0,
                   ElaIconType::Gear);

    aboutPage = new AboutPage ();
    QString aboutPageKey;
    addFooterNode (tr ("About"), aboutPage, aboutPageKey, 0,
                   ElaIconType::CircleInfo);

    loginPage = new LoginPage ();
    loginPage->setStyleSheet ("background-color:rgb(231, 252, 249);"
                              "QWidget { background-color: transparent; }");

    myPage = new MyPage ();
}

void MainWindow::initConnections ()
{
    connect (this, &ElaWindow::userInfoCardClicked, this,
             [this] ()
             {
                 if (!AccountManager::getInstance ().isLoggedIn ())
                 {
                     // show login page if not logged in
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

    connect (&AccountManager::getInstance (), &AccountManager::logoutSuccessful,
             this, &MainWindow::onLogoutSuccessful);

    connect (&AccountManager::getInstance (),
             &AccountManager::changeAvatarSuccessful, this,
             [this] (const QString &newAvatarPath)
             {
                 setUserInfoCardPixmap (QPixmap (newAvatarPath));
                 myPage->setAvatar (QPixmap (newAvatarPath));
             });
}

void MainWindow::onLoginSuccessful (const QString &username)
{
    setUserInfoCardTitle (username);

    setUserInfoCardSubTitle (
        QString ("%1").arg (AccountManager::getInstance ().getEmail ()));

    setUserInfoCardPixmap (
        QPixmap (AccountManager::getInstance ().getAvatarPath ()));

    myPage->setEnabled (true);

    myPage->usernameLineEdit->setText (username);

    myPage->emailLineEdit->setText (AccountManager::getInstance ().getEmail ());

    myPage->setAvatar (
        QPixmap (AccountManager::getInstance ().getAvatarPath ()));
}

void MainWindow::onLogoutSuccessful ()
{
    setUserInfoCardTitle (tr ("User"));
    setUserInfoCardSubTitle (tr ("Click to login"));

    setUserInfoCardPixmap (QPixmap (Constants::Resources::DEFAULT_USER_AVATAR));

    myPage->setEnabled (false);

    myPage->usernameLineEdit->clear ();
    myPage->emailLineEdit->clear ();
}
