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

    // This will change the UI by calling the slot enableHistorySearchListUI
    // (bool enabled) in HistoryPage
}

void UIController::notifySearchHistoryUpdated ()
{
    emit searchHistoryUpdated ();
}

void UIController::initConnections ()
{

    // Building...
    connect (&AppSettingModel::getInstance (),
             &AppSettingModel::historySearchListEnabledChanged, this,
             &UIController::enableHistorySearchListUI);
}