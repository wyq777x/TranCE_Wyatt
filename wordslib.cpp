#include "wordslib.h"
#include "ui_wordslib.h"
#include <QFile>
#include <QMessageBox>

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



}

wordslib::~wordslib()
{
    delete ui;
}
QTableWidgetItem* wordslib::getTableItem(int row, int column)
{
    return ui->wordstable->item(row, column);
}

void wordslib::on_pushButton_clicked()
{
    QFile file(":/words.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("警告"), tr("无法打开文件"));
        return;
    }

    QTextStream in(&file);
    int rowf=0;
    while (!in.atEnd()) {

        QString line = in.readLine();
        QStringList wordAndTranslation = line.split(" "); // 假设英文单词和中文翻译之间用制表符分隔

        if (wordAndTranslation.size() != 2) {
            continue; // 如果行格式不正确，跳过
        }



        QTableWidgetItem *englishItem = new QTableWidgetItem(wordAndTranslation[0]);
        QTableWidgetItem *chineseItem = new QTableWidgetItem(wordAndTranslation[1]);

        ui->wordstable->setItem(rowf, 0, englishItem); // 假设英文列在第0列
        ui->wordstable->setItem(rowf, 1, chineseItem); // 假设中文列在第1列
        rowf++;
    }

    file.close();
}

