#include "recitewords.h"
#include "wordslib.h"
#include "ui_recitewords.h"
#include <QTableWidget>
extern int mark;//外部引用分数
int row=0,column=0;//单词表格行和列
recitewords::recitewords(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::recitewords)
{
    ui->setupUi(this);
    this->setWindowTitle("背单词");
    QString iconpath=":/image/learneng.ico";
    QIcon icon(iconpath);
    this->setWindowIcon(icon);
    QStringList horizontalheader;
    horizontalheader << "英文";//1列头
    horizontalheader << "中文";//2列头
 wordsLib = new wordslib(this);//对应前面头文件里的对象,用于在背单词窗口中使用词库窗口的组件
    wrongWords=new wrongwords(this);//在背单词窗口中使用错词本组件

}

recitewords::~recitewords()
{
    delete ui;
}


void recitewords::on_btnstart_clicked()//开始按钮槽函数
{
    QAbstractButton* button = wordsLib->findChild<QAbstractButton*>("pushButton");
    if (button) {
        button->click();
    } else {
        QMessageBox::warning(this, tr("错误"), tr("无法找到按钮"));
    }
    QTableWidgetItem *item = wordsLib->getTableItem(row, column+1);
    if (!item) {
        // 如果item是nullptr，显示一个错误消息并返回
        QMessageBox::warning(this, tr("错误"), tr("无法获取词库中的单词"));
        return;
    }
    ui->lcdmark->display(mark);//显示实时分数
    QString chinese=item->text();//作为背单词时的中文提示
        ui->chineseline->setText(chinese);
}


void recitewords::on_btncommit_clicked()//提交按钮
{
    int rowrecite=0;
    int columnrecite=0;
    QTableWidgetItem *item = wordsLib->getTableItem(row, column);

    if (!item) {
        // 如果item是nullptr，显示一个错误消息并返回
        QMessageBox::warning(this, tr("错误"), tr("无法获取词库中的单词"));
        return;
    }
    QString b= item->text();
    QString a=ui->yourspell->toPlainText();
    if(a==b){
        mark+=10;//答对+10
        QMessageBox::information(this,tr("恭喜"),tr("答对,分数+10"));
    }
    else
    {mark-=10;//答错-10
        QMessageBox::information(this,tr("Oops"),tr("错误,分数-10"));
            // 复制wordstable的内容到wrongtable
        QTableWidgetItem *itemEnglish = wordsLib->getTableItem(row, 0);
        QTableWidgetItem *itemChinese = wordsLib->getTableItem(row, 1);
        if (!itemEnglish || !itemChinese) {
            // 如果itemEnglish或itemChinese是nullptr，显示一个错误消息并返回
            QMessageBox::warning(this, tr("错误"), tr("无法获取词库中的单词"));
            return;
        }
        QTableWidgetItem *wrongItemEnglish = new QTableWidgetItem(itemEnglish->text());
        QTableWidgetItem *wrongItemChinese = new QTableWidgetItem(itemChinese->text());
        wrongWords->setItem(row, 0, wrongItemEnglish);
        wrongWords->setItem(row, 1, wrongItemChinese);

        wrongWords->show();  // 显示wrongwords窗口
    }
    row++;
    ui->lcdmark->display(mark);//提交以后更新实时分数

    // 更新item到下一行的中文元素
    item = wordsLib->getTableItem(row, column+1);
    if (item) {
        QString chinese=item->text();//作为背单词时的中文提示
        ui->chineseline->setText(chinese);
    }
    ui->yourspell->clear();//清空输入框
}


void recitewords::on_btnclear_clicked()
{
    ui->yourspell->clear();//清空
}

