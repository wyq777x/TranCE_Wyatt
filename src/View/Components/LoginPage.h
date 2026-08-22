#pragma once

#include "View/TempPage.h"

class ElaLineEdit;
class ElaPushButton;
class QHBoxLayout;
class QLabel;
class QPaintEvent;
class QVBoxLayout;
class QWidget;
class RegisterPage;

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