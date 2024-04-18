#ifndef WRONGWORDS_H
#define WRONGWORDS_H
#include "wordslib.h"
#include <QDialog>
#include <QIcon>
#include <QWidget>
#include <QMessageBox>
namespace Ui {
class wrongwords;
}

class wrongwords : public QDialog
{
    Q_OBJECT

public:
    explicit wrongwords(QWidget *parent = nullptr);
    ~wrongwords();
    //下面是将词库里的错词移入错词本
    void addRow(QTableWidgetItem *item,int column);//设置错词本插入一整列的重载
    void addRow(QTableWidgetItem *itemEnglish, QTableWidgetItem *itemChinese);//回答错误移入错词本的重载
    void addRow();//默认重载
    void setItem(int row,int column,QTableWidgetItem*item);
private slots:

private:
    Ui::wrongwords *ui;
    wordslib *wordsLib;
};

#endif // WRONGWORDS_H
