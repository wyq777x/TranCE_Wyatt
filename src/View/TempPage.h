#pragma once

#include <ElaScrollPage.h>
#include <QString>
#include <functional>

class ElaLineEdit;
class QWidget;

class TempPage : public ElaScrollPage
{
    Q_OBJECT
public:
    explicit TempPage (QWidget *parent = nullptr);
    ~TempPage ();

    // map language to code
    QString mapLang (const QString &lang)
    {
        if (lang == "Chinese" || lang == "中文" || lang == "汉语")
            return "zh";
        if (lang == "English" || lang == "英语" || lang == "英文")
            return "en";
        return lang;
    };

    void showDialog (const QString &title, const QString &message,
                     bool closeParentOnAccept = false);

    void showDialog (const QString &title, const QString &message,
                     std::function<void ()> onConfirm,
                     std::function<void ()> onReject = nullptr);

    void showDialog (const QString &title, std::function<void ()> onConfirm,
                     std::function<void ()> onReject = nullptr);

protected:
    ElaLineEdit *oldPasswordLineEdit;
    ElaLineEdit *newPasswordLineEdit;
};