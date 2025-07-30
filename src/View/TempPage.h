#pragma once

#include "Utility/ClickableWidget.h"
#include "Utility/Constants.h"
#include <ElaComboBox.h>
#include <ElaContentDialog.h>
#include <ElaFlowLayout.h>
#include <ElaIcon.h>
#include <ElaIconButton.h>
#include <ElaLineEdit.h>
#include <ElaListView.h>
#include <ElaPushButton.h>
#include <ElaRadioButton.h>
#include <ElaScrollArea.h>
#include <ElaScrollPage.h>
#include <ElaTableView.h>
#include <ElaTheme.h>
#include <ElaToggleButton.h>
#include <ElaToggleSwitch.h>
#include <ElaWidget.h>
#include <QAction>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFrame>
#include <QGraphicsBlurEffect>
#include <QLabel>
#include <QPainter>
#include <QPropertyAnimation>
#include <QStackedLayout>
#include <QStringList>
#include <QStringListModel>
#include <QVBoxLayout>
#include <functional>


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

    void showDialog (const QString &title, const QString &message);

    void showDialog (const QString &title, const QString &message,
                     std::function<void ()> onConfirm,
                     std::function<void ()> onReject = nullptr);

    void showDialog (const QString &title, std::function<void ()> onConfirm,
                     std::function<void ()> onReject = nullptr);

protected:
    ElaLineEdit *oldPasswordLineEdit;
    ElaLineEdit *newPasswordLineEdit;
};