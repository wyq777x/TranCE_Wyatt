#pragma once
#include "View/TempPage.h"

class AboutPage : public TempPage
{
    Q_OBJECT
public:
    explicit AboutPage (QWidget *parent = nullptr);

private:
    void initUI ();
    void initConnections ();

    // UI components
    QWidget *centralWidget;
    QVBoxLayout *aboutPageLayout;
    QHBoxLayout *titleLayout;
    QVBoxLayout *detailsLayout;
    QLabel *logoLabel;
    QLabel *titleLabel;
    QLabel *descriptionLabel;
    ElaPushButton *repoButton;
    ElaPushButton *licenseButton;
};