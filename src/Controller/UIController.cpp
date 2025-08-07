#include "UIController.h"
#include "Controller/AccountManager.h"
#include "Model/AppSettingModel.h"
#include "View/Components/QuizCard.h"
#include "View/Components/WordCard.h"

void UIController::showWordCard (WordEntry &entry, QWidget *parent)
{
    WordCard *wordCard = WordCard::getInstance (parent);
    wordCard->setWordEntry (entry);

    wordCard->setAdd2FavoritesButtonEnabled (
        AccountManager::getInstance ().isLoggedIn ());

    wordCard->show ();
}

QuizCard *UIController::showQuizCard (WordEntry &entry, QWidget *parent)
{
    QuizCard *quizCard = QuizCard::getInstance (parent);
    quizCard->setWordEntry (entry);
    quizCard->setAdd2FavoritesButtonEnabled (
        AccountManager::getInstance ().isLoggedIn ());
    quizCard->fillReciteOptions ();
    quizCard->shuffleReciteOptions ();
    quizCard->setReciteOptions (quizCard->getReciteOptions ());

    quizCard->show ();

    return quizCard;
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

    connect (&AppSettingModel::getInstance (),
             &AppSettingModel::historySearchListEnabledChanged, this,
             &UIController::enableHistorySearchListUI);
}