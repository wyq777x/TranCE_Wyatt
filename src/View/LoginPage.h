#pragma once
#include "Controller/AccountManager.h"
#include "ElaFlowLayout.h"
#include "ElaLineEdit.h"
#include "ElaPushButton.h"
#include "TempPage.h"
#include "View/RegisterPage.h"

class LoginPage : public TempPage
{
    Q_OBJECT
public:
    explicit LoginPage (QWidget *parent = nullptr);
    void paintEvent (QPaintEvent *event) override;

private:
    RegisterPage *registerPage;
};