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
    void initUI ();
    void initConnections ();

    ElaLineEdit *usernameLineEdit;
    ElaLineEdit *passwordLineEdit;
    ElaPushButton *loginButton;
    ElaPushButton *registerButton;
    RegisterPage *registerPage;

    // UI components
    QWidget *centralWidget;
    QVBoxLayout *loginPageLayout;
    QLabel *titleLabel;
    QHBoxLayout *loginButtonLayout;
};