#pragma once
#include "ElaLineEdit.h"
#include "View/TempPage.h"

class MyPage : public TempPage
{
    Q_OBJECT
public:
    explicit MyPage (QWidget *parent = nullptr);

    ElaLineEdit *usernameLineEdit;
    ElaLineEdit *emailLineEdit;
    void setAvatar (const QPixmap &avatar) { avatarLabel->setPixmap (avatar); }
signals:
    void usernameChanged (const QString &newUsername);
    void emailChanged (const QString &newEmail);

private:
    QLabel *avatarLabel;
};
