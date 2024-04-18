#ifndef RECITEWORDS_H
#define RECITEWORDS_H
#include "wordslib.h"
#include "wrongwords.h"
#include <QDialog>
#include <QIcon>
#include <QWidget>
#include <QMessageBox>

namespace Ui {
class recitewords;
}

class recitewords : public QDialog
{
    Q_OBJECT

public:
    explicit recitewords(QWidget *parent = nullptr);
    ~recitewords();

private slots:
    void on_btnstart_clicked();

    void on_btncommit_clicked();

    void on_btnclear_clicked();

private:
    //后面构造复制用
    Ui::recitewords *ui;
    wordslib *wordsLib;
    wrongwords *wrongWords;
};

#endif // RECITEWORDS_H
