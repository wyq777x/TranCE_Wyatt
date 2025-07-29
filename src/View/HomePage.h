#pragma once
#include "View/TempPage.h"

class HomePage : public TempPage
{
    Q_OBJECT
public:
    explicit HomePage (QWidget *parent = nullptr);

private:
    void initUI ();
    void initConnections ();

    // UI components
    QWidget *centralWidget;
    QVBoxLayout *homePageLayout;
    QHBoxLayout *langLayout;
    QHBoxLayout *searchLayout;
    QHBoxLayout *searchModeLayout;
    QHBoxLayout *randomRecommendationLayout;

    ElaComboBox *LangComboBox_left;
    ElaComboBox *LangComboBox_right;
    ElaIconButton *swapButton;
    ElaToggleButton *searchOnline;
    ElaLineEdit *lineEdit;
    ElaListView *suggestionsList;
    QStringListModel *suggestionModel;
    QStringListModel *historySearchModel;
    QAction *searchAction;
    QAction *clearAction;
    ElaRadioButton *searchMode_precise;
    ElaRadioButton *searchMode_fuzzy;
    QLabel *randomRecommendationLabel;
    ElaPushButton *recommendWordButton;
};