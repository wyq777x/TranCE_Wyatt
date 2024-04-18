#include "logindialog.h"
#include "ui_logindialog.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QIcon>

logindialog::logindialog(QWidget *parent)//构造logindialog窗口对象
    : QDialog(parent)//qt会话框
    , ui(new Ui::logindialog)
{
    ui->setupUi(this);
    this->setWindowTitle("用户注册登录(Login&Register)");//设置窗口标题
    QString iconpath=":/image/learneng.ico";//设图标路径
    QIcon icon(iconpath);
    this->setWindowIcon(icon);//设图标

    ui->userline->setPlaceholderText("请输入用户名");
    ui->passwordline->setPlaceholderText("请输入密码");//在不进行任何输入时候的文本显示

}

logindialog::~logindialog()
{
    delete ui;
}

void logindialog::on_btnlogin_clicked()//登录按钮槽函数
{

    Userinfo userinfo;  //用户数据
    QFile file ("user_info.txt");//用户数据存放地址
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text))//必须以只读或文本模式打开
    {
        QMessageBox::critical(this,tr("Error"),tr("找不到文件读取"));
        return;
    }

    QTextStream in(&file);//从txt里抽出给流
    QString storedUsername = in.readLine().trimmed();
    QString storedPassword = in.readLine();
    QString inputUsername = ui->userline->text().trimmed();
    QString inputPassword = ui->passwordline->text();
    if (inputUsername == storedUsername && inputPassword == storedPassword) {//比对用户名和密码
        QMessageBox::information(this, tr("Success"), tr("登录成功"));
        accept();//传给main函数一个接受信号,然后在main函数里通过show()方式打开MainWindow
    } else {
        QMessageBox::critical(this, tr("Error"), tr("用户名或密码错误"));//错误提示
        return;
    }


}

void logindialog::on_btnregister_clicked()//注册按钮槽函数
{
    Userinfo userinfo;
    userinfo.username = ui->userline->text().trimmed();
    userinfo.password = ui->passwordline->text();

    QFile file("user_info.txt");
    // 首先检查文件是否存在，如果不存在则可以直接注册
    if(!file.exists()) {
        // 文件不存在，可以注册新用户
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(this, tr("Error"), tr("无法打开并写入文件"));
            return;
        }
        QTextStream out(&file);
        out << userinfo.username << "\n";
        out << userinfo.password << "\n";
        file.close();
        QMessageBox::information(this, tr("Success"), tr("用户数据已储存,请登录"));
        return;
    }

    // 如果文件存在，检查用户名是否已存在
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Error"), tr("无法打开文件读取"));
        return;
    }
    QTextStream in(&file);
    QString line;
    while(!in.atEnd()) {
        line = in.readLine().trimmed();
        if(line == userinfo.username) {
            QMessageBox::critical(this, tr("Error"), tr("用户名已存在，请选择其他用户名"));
            file.close();
            return;
        }
        // 跳过密码行
        if(!in.atEnd()) in.readLine();
    }
    file.close();

    // 用户名不存在，可以注册新用户
    if(!file.open(QIODevice::Append | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Error"), tr("无法打开并写入文件"));
        return;
    }
    QTextStream out(&file);
    out << userinfo.username << "\n";
    out << userinfo.password << "\n";
    file.close();
    QMessageBox::information(this, tr("Success"), tr("用户数据已储存,请登录"));
}


