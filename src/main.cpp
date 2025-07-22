#include "Controller/AccountManager.h"
#include "Controller/DbManager.h"
#include "MainWindow.h"
#include "Model/UserModel.h"
#include <ElaApplication.h>
#include <QApplication>
#include <QDir>
#include <QMainWindow>

int main (int argc, char *argv[])
{
    QApplication a (argc, argv);

    ElaApplication::getInstance ()->init ();

    auto &dbManager = DbManager::getInstance ();
    auto initTask = dbManager.initializeAsync ();

    MainWindow w;
    w.show ();

    AccountManager::getInstance ().setUserModel (&UserModel::getInstance ());
    return a.exec ();
}