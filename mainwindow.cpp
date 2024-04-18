#include "mainwindow.h"
#include "ui_mainwindow.h"



int n=0;
int mark=0;//分数
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);  //创建窗口UI
    setWindowTitle("learnENG");//设置标题
    QString iconpath=":/image/learneng.ico";
    QIcon icon(iconpath);//将上方的图标路径传给QIcon变量以设置图标
    this->setWindowIcon(icon);


}

MainWindow::~MainWindow()//析构函数,关闭窗口后析构UI对象,防止持续占用内存
{
    delete ui;
}
//以下是信号槽部分，均为几个QPushButton的槽函数

void MainWindow::on_btnsign_clicked()//打卡按钮槽函数
{
    //打卡按钮的槽函数
    n++;
//下面是一个LCD计数器
    ui->lcdsigntime->display(n);//变更LCD计数器显示数字
    QMessageBox::information(this, tr("Success"), tr("打卡成功"));
}


void MainWindow::on_btnexit_clicked()//退出程序按钮
{//退出程序
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("退出前的确认"), tr("你想要打卡吗?"),
                                  QMessageBox::Yes|QMessageBox::No);

    // 根据用户的选择进行处理
    if (reply == QMessageBox::Yes) {
        // 用户选择了"Yes"，点击 btnsign 按钮
        QPushButton *btnsign = this->findChild<QPushButton *>("btnsign");
        if (btnsign) {
            btnsign->click(); // 模拟点击 btnsign 按钮
        }
    }

    QApplication::quit();//QApplication的退出,比exit()好
}


void MainWindow::on_btnlastmark_clicked()
{

    //提示上次背单词的成绩
    std::string a=std::to_string(mark);
    std::string b="你的成绩是: "+a;//两个字符串拼接
    QString c =QString::fromStdString(b);
    QMessageBox::information(this,tr("成绩"),c);
}


void MainWindow::on_btnwrongword_clicked()//错词本按钮
{
    //进入错词本
    wrongwords.show();//显示错词本窗口
    wrongwords.raise();//错词本窗口显示在上层

}


void MainWindow::on_btntowordslib_clicked()
{
    //进入词库
    wordslib.show();
    wordslib.raise();

}


void MainWindow::on_btnrecite_clicked()
{
    //进入背单词界面
    recitewords.show();
    recitewords.raise();
}

