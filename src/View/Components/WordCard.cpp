#include "Components/WordCard.h"

WordCard::WordCard (QWidget *parent) : TempPage (parent)
{
    setMinimumSize (500, 300);
    setMaximumSize (500, 300);
    setStyleSheet (
        "background-color: #f0f0f0; font-family: 'Noto Sans', sans-serif;");
}