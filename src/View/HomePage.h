#pragma once
#include "View/TempPage.h"

class HomePage : public TempPage
{
    Q_OBJECT
public:
    explicit HomePage (QWidget *parent = nullptr);

    // map language to code
    QString mapLang (const QString &lang)
    {
        if (lang == "Chinese" || lang == "中文" || lang == "汉语")
            return "zh";
        if (lang == "English" || lang == "英语" || lang == "英文")
            return "en";
        return lang;
    };

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
    QAction *searchAction;
    QAction *clearAction;
    ElaRadioButton *searchMode_precise;
    ElaRadioButton *searchMode_fuzzy;
    QLabel *randomRecommendationLabel;
    ElaPushButton *recommendWordButton;
};