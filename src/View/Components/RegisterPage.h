#pragma once

#include "View/TempPage.h"

class RegisterPage : public TempPage
{
    Q_OBJECT
public:
    explicit RegisterPage (QWidget *parent = nullptr);
    void paintEvent (QPaintEvent *event) override;

private:
    void initUI ();
    void initConnections ();

    QWidget *centralWidget;
    QVBoxLayout *registerPageLayout;
    QHBoxLayout *registerButtonLayout;
    QLabel *titleLabel;
    ElaLineEdit *usernameLineEdit;
    ElaLineEdit *passwordLineEdit;
    ElaLineEdit *confirmPasswordLineEdit;
    ElaPushButton *registerButton;
    ElaPushButton *cancelButton;
};