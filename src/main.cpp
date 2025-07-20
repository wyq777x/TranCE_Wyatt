#include "MainWindow.h"
#include <Controller/AccountManager.h>
#include <Controller/DbManager.h>
#include <ElaApplication.h>
#include <Model/UserModel.h>
#include <QApplication>
#include <QDir>
#include <QLocale>
#include <QMainWindow>
#include <QTranslator>
#include <Utility/AsyncTask.h>


int main (int argc, char *argv[])
{
    QApplication a (argc, argv);

    // Initialize internationalization
    QTranslator translator;
    QString locale = QLocale::system ().name ();

    // Try to load translation file based on system locale
    if (translator.load ("TranCE_Wyatt_" + locale, ":/translations"))
    {
        a.installTranslator (&translator);
    }
    else
    {
        // Fallback to English if translation not found
        if (translator.load ("TranCE_Wyatt_en_US", ":/translations"))
        {
            a.installTranslator (&translator);
        }
    }

    ElaApplication::getInstance ()->init ();

    auto &dbManager = DbManager::getInstance ();
    auto initTask = dbManager.initializeAsync ();

    MainWindow w;
    w.show ();

    AccountManager::getInstance ().setUserModel (&UserModel::getInstance ());
    return a.exec ();
}