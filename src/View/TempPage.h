#pragma once
#include "Controller/AccountManager.h"
#include <ElaComboBox.h>
#include <ElaContentDialog.h>
#include <ElaFlowLayout.h>
#include <ElaIcon.h>
#include <ElaIconButton.h>
#include <ElaLineEdit.h>
#include <ElaListView.h>
#include <ElaPushButton.h>
#include <ElaScrollArea.h>
#include <ElaScrollPage.h>
#include <ElaTableView.h>
#include <ElaTheme.h>
#include <ElaToggleSwitch.h>
#include <ElaWidget.h>
#include <QAction>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QFrame>
#include <QGraphicsBlurEffect>
#include <QLabel>
#include <QPainter>
#include <QStackedLayout>
#include <QVBoxLayout>
class TempPage : public ElaScrollPage
{
    Q_OBJECT
public:
    explicit TempPage (QWidget *parent = nullptr);
    ~TempPage ();

    void showDialog (const QString &title, const QString &message);
};