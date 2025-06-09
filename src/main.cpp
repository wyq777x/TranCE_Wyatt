#include "MainWindow.h"
#include <Controller/AccountManager.h>
#include <ElaApplication.h>
#include <Model/DbModel.h>
#include <Model/UserModel.h>
#include <QApplication>
#include <QMainWindow>
#include <Utility/AsyncTask.h>

int main (int argc, char *argv[])
{
    QApplication a (argc, argv);
    ElaApplication::getInstance ()->init ();

    auto &db = DbModel::getInstance ();
    auto initTask = db.initializeAsync ();

    MainWindow w;
    w.show ();

    AccountManager::getInstance ().setUserModel (&UserModel::getInstance ());
    return a.exec ();
}