#pragma once

#include "View/TempPage.h"

class MyPage : public TempPage
{
    Q_OBJECT
public:
    explicit MyPage (QWidget *parent = nullptr);

    ElaLineEdit *usernameLineEdit;
    ElaLineEdit *emailLineEdit;
    ElaLineEdit *oldPasswordLineEdit;
    ElaLineEdit *newPasswordLineEdit;
    void setAvatar (const QPixmap &avatar) { avatarLabel->setPixmap (avatar); }

signals:
    void usernameChanged (const QString &newUsername);
    void emailChanged (const QString &newEmail);

private:
    void initUI ();
    void initConnections ();

    QWidget *centralWidget;
    QVBoxLayout *userProfileLayout;
    QLabel *avatarLabel;
    ElaPushButton *changeAvatarButton;
    ElaPushButton *changeUsernameButton;
    ElaPushButton *changePasswordButton;
    ElaPushButton *changeEmailButton;
    QPushButton *logoutButton;
};
