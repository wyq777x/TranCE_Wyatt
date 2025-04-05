#pragma once
#include "TempPage.h"
#include <QPainter>
class LoginPage : public TempPage
{
    Q_OBJECT
public:
    explicit LoginPage (QWidget *parent = nullptr);
    void paintEvent (QPaintEvent *event) override;
};