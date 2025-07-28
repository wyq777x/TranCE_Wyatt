#include "UIController.h"
#include "AppSettingModel.h"
#include "View/Components/WordCard.h"

void UIController::showWordCard (WordEntry &entry, QWidget *parent)
{
    WordCard *wordCard = WordCard::getInstance (parent);
    wordCard->setWordEntry (entry);
    wordCard->show ();
}

void UIController::enableHistorySearchListUI (bool enabled)
{
    emit historySearchListUIChanged (enabled);
}

void UIController::initConnections ()
{

    // Building...
    connect (&AppSettingModel::getInstance (),
             &AppSettingModel::historySearchListEnabledChanged, this,
             &UIController::enableHistorySearchListUI);
}