#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H
#include <QString>
#include <QDialog>


class Userinfo
{
public:
    QString username;
    QString password;
};

namespace Ui {
class logindialog;
}

class logindialog : public QDialog
{
    Q_OBJECT

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
