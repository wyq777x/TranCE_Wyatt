#include "UIController.h"
#include "View/Components/WordCard.h"

void UIController::showWordCard (WordEntry &entry, QWidget *parent)
{
    WordCard *wordCard = WordCard::getInstance (parent);
    wordCard->setWordEntry (entry);
    wordCard->show ();
}