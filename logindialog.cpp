#include "logindialog.h"
#include "ui_logindialog.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QIcon>

logindialog::logindialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::logindialog)
{
    ui->setupUi(this);
    this->setWindowTitle("用户注册登录(Login&Register)");
    QString iconpath=":/image/learneng.ico";
    QIcon icon(iconpath);
    this->setWindowIcon(icon);

    ui->userline->setPlaceholderText("请输入用户名");
    ui->passwordline->setPlaceholderText("请输入密码");

}

logindialog::~logindialog()
{
    delete ui;
}

void logindialog::on_btnlogin_clicked()
{

    Userinfo userinfo;  //用户数据
    QFile file ("user_info.txt");//用户数据存放地址
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        QMessageBox::critical(this,tr("Error"),tr("找不到文件读取"));
        return;
    }

    QTextStream in(&file);//从txt里抽出给流
    QString storedUsername = in.readLine().trimmed();
    QString storedPassword = in.readLine();
    QString inputUsername = ui->userline->text().trimmed();
    QString inputPassword = ui->passwordline->text();
    if (inputUsername == storedUsername && inputPassword == storedPassword) {
        QMessageBox::information(this, tr("Success"), tr("登录成功"));
        accept();
    } else {
        QMessageBox::critical(this, tr("Error"), tr("用户名或密码错误"));
        return;
    }


}

void logindialog::on_btnregister_clicked()
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


