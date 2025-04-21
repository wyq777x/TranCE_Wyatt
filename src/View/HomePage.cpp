#include "HomePage.h"
#include "Def.h"
#include "ElaComboBox.h"
#include "ElaFlowLayout.h"
#include "ElaIcon.h"
#include "ElaLineEdit.h"
#include "ElaPushButton.h"
#include "ElaToggleButton.h"
#include <qboxlayout.h>
#include <qnamespace.h>

HomePage::HomePage (QWidget *parent) : TempPage (parent)
{
    setWindowTitle ("Home");

    auto *centralWidget = new QWidget (this);
    centralWidget->setWindowTitle ("Home");

    QVBoxLayout *homePageLayout = new QVBoxLayout (centralWidget);
    homePageLayout->setAlignment (Qt::AlignHCenter | Qt::AlignTop);
    homePageLayout->setSpacing (20);

    ElaComboBox *LangComboBox_left = new ElaComboBox (centralWidget);
    ElaComboBox *LangComboBox_right = new ElaComboBox (centralWidget);
    LangComboBox_left->setStyleSheet (
        "QComboBox {"
        "    color: black;" // set default text color as black
        "    font-family: 'Noto Sans';"
        "}"
        "QComboBox QAbstractItemView {"
        "    color: black;" // set text color of dropdown list as black
        "}");

    LangComboBox_right->setStyleSheet (
        "QComboBox {"
        "    color: black;" // set default text color as black
        "    font-family: 'Noto Sans';"
        "}"
        "QComboBox QAbstractItemView {"
        "    color: black;" // set text color of dropdown list as black
        "}");

    ElaIconButton *swapButton =
        new ElaIconButton (ElaIconType::SwapArrows, centralWidget);
    swapButton->setBorderRadius (3);

    QHBoxLayout *langLayout = new QHBoxLayout (centralWidget);
    langLayout->setAlignment (Qt::AlignHCenter);
    langLayout->setSpacing (20);
    langLayout->addWidget (LangComboBox_left);
    langLayout->addWidget (swapButton);
    langLayout->addWidget (LangComboBox_right);

    homePageLayout->addLayout (langLayout);
    homePageLayout->setAlignment (langLayout, Qt::AlignHCenter);

    LangComboBox_left->addItem ("English");
    LangComboBox_left->addItem ("Chinese");

    LangComboBox_right->addItem ("Chinese");
    LangComboBox_right->addItem ("English");

    ElaToggleButton *searchOnline = new ElaToggleButton (centralWidget);
    searchOnline->setText ("Search\nOnline");
    searchOnline->setMinimumHeight (60);
    searchOnline->setMaximumHeight (60);
    searchOnline->setBorderRadius (6);
    searchOnline->setIsToggled (false); // default is false

    ElaLineEdit *lineEdit = new ElaLineEdit (centralWidget);
    lineEdit->setPlaceholderText ("Search...");
    lineEdit->setMinimumHeight (70);
    lineEdit->setMaximumHeight (70);
    lineEdit->setMinimumWidth (550);
    lineEdit->setBorderRadius (20);

    QAction *searchAction = lineEdit->addAction (
        ElaIcon::getInstance ()->getElaIcon (ElaIconType::MagnifyingGlass, 512),
        QLineEdit::LeadingPosition);
    QAction *clearAction = lineEdit->addAction (
        ElaIcon::getInstance ()->getElaIcon (ElaIconType::Xmark, 512),
        QLineEdit::TrailingPosition);
    searchAction->setToolTip ("Search");
    clearAction->setToolTip ("Clear");

    QHBoxLayout *searchLayout = new QHBoxLayout (centralWidget);
    searchLayout->setAlignment (Qt::AlignHCenter);
    searchLayout->setSpacing (20);
    searchLayout->addWidget (lineEdit);
    searchLayout->addWidget (searchOnline);
    homePageLayout->addLayout (searchLayout);

    addCentralWidget (centralWidget, true, true, 0);
}