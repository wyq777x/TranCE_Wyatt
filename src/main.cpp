#include <ElaApplication.h>
#include "ElaWindow.h"
#include "MainWindow.h"
#include <QApplication>
#include <QMainWindow>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ElaApplication::getInstance()->init();
    MainWindow w;
    w.show();
    return a.exec();
}