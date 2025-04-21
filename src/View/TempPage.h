#pragma once
#include <ElaFlowLayout.h>
#include <ElaIconButton.h>
#include <ElaLineEdit.h>
#include <ElaPushButton.h>
#include <ElaScrollArea.h>
#include <ElaScrollPage.h>
#include <ElaTheme.h>
#include <ElaToggleSwitch.h>
#include <QLabel>
#include <QVBoxLayout>
class TempPage : public ElaScrollPage
{
    Q_OBJECT
public:
    explicit TempPage (QWidget *parent = nullptr);
    ~TempPage ();
};