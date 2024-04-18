#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H
#include <QString>
#include <QDialog>


class Userinfo//存储用户登录信息
{
public:
    QString username;//用户名
    QString password;//密码
};

namespace Ui {
class logindialog;
}

class logindialog : public QDialog
{
    Q_OBJECT//Qt宏

public:
    explicit logindialog(QWidget *parent = nullptr);
    ~logindialog();

private slots:
    void on_btnlogin_clicked();

    void on_btnregister_clicked();

private:
    Ui::logindialog *ui;
};

#endif // LOGINDIALOG_H
