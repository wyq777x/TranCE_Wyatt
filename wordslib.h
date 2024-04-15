#ifndef WORDSLIB_H
#define WORDSLIB_H

#include <QDialog>

namespace Ui {
class wordslib;
}

class wordslib : public QDialog
{
    Q_OBJECT

public:
    explicit wordslib(QWidget *parent = nullptr);
    ~wordslib();

private:
    Ui::wordslib *ui;
};

#endif // WORDSLIB_H
