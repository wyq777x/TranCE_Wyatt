#ifndef WORDSLIB_H
#define WORDSLIB_H
#include <QTableWidgetItem>
#include <QDialog>
extern int row,column;
namespace Ui {
class wordslib;
}

class wordslib : public QDialog
{
    Q_OBJECT

public:
    explicit wordslib(QWidget *parent = nullptr);
    ~wordslib();
    QTableWidgetItem* getTableItem(int row, int column);//获取表格对象,到时候获取对象的text()
    void setItem(int row, int column, QTableWidgetItem *item);

private slots:
    void on_pushButton_clicked();

private:
    Ui::wordslib *ui;
};

#endif // WORDSLIB_H
