#pragma once
#include "Controller/AccountManager.h"
#include "TempPage.h"
class RegisterPage : public TempPage
{
    Q_OBJECT
public:
    explicit RegisterPage (QWidget *parent = nullptr);
    void paintEvent (QPaintEvent *event) override;
};