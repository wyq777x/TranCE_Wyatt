#pragma once

#include "View/Components/RegisterPage.h"
#include "View/TempPage.h"

class LoginPage : public TempPage
{
    Q_OBJECT
public:
    explicit LoginPage (QWidget *parent = nullptr);
    void paintEvent (QPaintEvent *event) override;

private:
    RegisterPage *registerPage;
};