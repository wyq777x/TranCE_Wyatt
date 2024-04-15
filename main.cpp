#include "mainwindow.h"
#include "logindialog.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);   //创建一个Qt Application组件
    MainWindow w;    //创建一个主窗口对象
    logindialog loginpage;  //登录页对象
    if(loginpage.exec()==QDialog::Accepted)
    {
    w.show();    //从登录页入口至MainWindow
    return a.exec();
}
    else return 1;
}
