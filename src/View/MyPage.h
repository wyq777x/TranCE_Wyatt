#pragma once
#include "ElaLineEdit.h"
#include "TempPage.h"

class MyPage : public TempPage
{
    Q_OBJECT
public:
    explicit MyPage (QWidget *parent = nullptr);

    ElaLineEdit *usernameLineEdit;
    ElaLineEdit *emailLineEdit;
};
