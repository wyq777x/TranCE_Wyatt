#include "wrongwords.h"
#include "ui_wrongwords.h"

wrongwords::wrongwords(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::wrongwords)
{
    ui->setupUi(this);
    setWindowTitle("错词本");
    QString iconpath=":/image/learneng.ico";
    QIcon icon(iconpath);
    this->setWindowIcon(icon);

    QStringList horizontalheader;
    horizontalheader << "英文";
    horizontalheader << "中文";
    ui->wrongtable->setHorizontalHeaderLabels(horizontalheader);
    ui->wrongtable->setItem(0, 0, new QTableWidgetItem("Some text"));
    ui->wrongtable->setItem(0, 1, new QTableWidgetItem("你好"));
}

wrongwords::~wrongwords()
{
    delete ui;
}
void wrongwords::addRow(QTableWidgetItem *item, int column)//添加列
{
    int row = ui->wrongtable->rowCount();
    ui->wrongtable->insertRow(row);
    ui->wrongtable->setItem(row, column, item);

}
void wrongwords::addRow(QTableWidgetItem *itemEnglish, QTableWidgetItem *itemChinese)
{
    int row = ui->wrongtable->rowCount();
    ui->wrongtable->setItem(row, 0, itemEnglish);
    ui->wrongtable->setItem(row, 1, itemChinese);
}

void wrongwords::setItem(int row, int column, QTableWidgetItem *item)
{
    ui->wrongtable->setItem(row, column, item);
}



