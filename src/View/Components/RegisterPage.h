#pragma once

#include "View/TempPage.h"

class RegisterPage : public TempPage
{
    Q_OBJECT
public:
    explicit RegisterPage (QWidget *parent = nullptr);
    void paintEvent (QPaintEvent *event) override;
};