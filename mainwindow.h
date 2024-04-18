#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QMessageBox>
#include "recitewords.h"
#include "wordslib.h"
#include "wrongwords.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    //下面三个都是为了后面在不同窗口之间数据拷贝用的,因为我的各个窗口的ui都是private成员
    recitewords recitewords;
    wordslib wordslib;
    wrongwords wrongwords;



private slots:
    void on_btnsign_clicked();

    void on_btnexit_clicked();

    void on_btnlastmark_clicked();

    void on_btnwrongword_clicked();

    void on_btntowordslib_clicked();

    void on_btnrecite_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
