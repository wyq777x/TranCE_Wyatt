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
    MainWindow w;
    w.show ();

    AccountManager::getInstance ().setUserModel (&UserModel::getInstance ());
    return a.exec ();
}