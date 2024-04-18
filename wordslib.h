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

private:
    Ui::wordslib *ui;
};

#endif // WORDSLIB_H
