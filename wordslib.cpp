#include "wordslib.h"
#include "ui_wordslib.h"

wordslib::wordslib(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::wordslib)
{
    ui->setupUi(this);
    setWindowTitle("词库");
    QString iconpath=":/image/learneng.ico";
    QIcon icon(iconpath);
    this->setWindowIcon(icon);
    QStringList horizontalheader;
    horizontalheader << "英文";
    horizontalheader << "中文";
    ui->wordstable->setHorizontalHeaderLabels(horizontalheader);

    ui->wordstable->setRowCount(200);
    ui->wordstable->setColumnCount(2);
    for (int i = 0; i < 200; ++i) {

            ui->wordstable->setItem(i, 0, new QTableWidgetItem("Some text"));
            ui->wordstable->setItem(i, 1, new QTableWidgetItem("你好"));
        }


}

wordslib::~wordslib()
{
    delete ui;
}
QTableWidgetItem* wordslib::getTableItem(int row, int column)
{
    return ui->wordstable->item(row, column);
}
