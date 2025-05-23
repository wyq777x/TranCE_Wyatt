#include "ElaWindow.h"
#include "MainWindow.h"
#include <Controller/AccountManager.h>
#include <ElaApplication.h>
#include <QApplication>
#include <QMainWindow>

int main (int argc, char *argv[])
{
    QApplication a (argc, argv);
    ElaApplication::getInstance ()->init ();

    auto &db = DbModel::getInstance ();
    db.initDBs ();
    MainWindow w;
    w.show ();

    AccountManager::getInstance ().setUserModel (&UserModel::getInstance ());
    return a.exec ();
}