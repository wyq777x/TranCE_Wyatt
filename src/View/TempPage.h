#pragma once
#include <ElaFlowLayout.h>
#include <ElaLineEdit.h>
#include <ElaPushButton.h>
#include <ElaScrollPage.h>
#include <ElaTheme.h>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>
class TempPage : public ElaScrollPage
{
    Q_OBJECT
public:
    explicit TempPage (QWidget *parent = nullptr);
    ~TempPage ();
};