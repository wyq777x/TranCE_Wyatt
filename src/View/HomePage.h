#pragma once
#include "TempPage.h"
#include "View/Components/WordCard.h"

class HomePage : public TempPage
{
    Q_OBJECT
public:
    explicit HomePage (QWidget *parent = nullptr);
};