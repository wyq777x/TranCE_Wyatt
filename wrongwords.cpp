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
}

wrongwords::~wrongwords()
{
    delete ui;
}




