#pragma once
#include "Utility/WordEntry.h"
#include <QObject>

class UIController : public QObject
{

    Q_OBJECT
public:
    static UIController &getInstance ()
    {
        static UIController instance;
        return instance;
    }

    void showWordCard (WordEntry &entry, QWidget *parent = nullptr);

private:
    UIController () = default;
    UIController (const UIController &) = delete;
    UIController &operator= (const UIController &) = delete;
    UIController (UIController &&) = delete;
    UIController &operator= (UIController &&) = delete;
};