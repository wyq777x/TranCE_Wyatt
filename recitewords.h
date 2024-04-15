#ifndef RECITEWORDS_H
#define RECITEWORDS_H

#include <QDialog>
#include <QIcon>

namespace Ui {
class recitewords;
}

class recitewords : public QDialog
{
    Q_OBJECT

public:
    explicit recitewords(QWidget *parent = nullptr);
    ~recitewords();

private:
    Ui::recitewords *ui;
};

#endif // RECITEWORDS_H
