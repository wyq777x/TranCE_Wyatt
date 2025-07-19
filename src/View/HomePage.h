#pragma once
#include "TempPage.h"
#include "View/Components/WordCard.h"

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
};