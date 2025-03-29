#pragma once
#include "TempPage.h"

class MyPage : public TempPage
{
    Q_OBJECT
public:
    explicit MyPage(QWidget *parent = nullptr);
};
