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
}

wordslib::~wordslib()
{
    delete ui;
}
