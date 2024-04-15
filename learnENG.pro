QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
INCLUDEPATH += $$PWD/FluentUI/src
SOURCES += \
    logindialog.cpp \
    main.cpp \
    mainwindow.cpp \
    recitewords.cpp \
    wordslib.cpp \
    wrongwords.cpp

HEADERS += \
    logindialog.h \
    mainwindow.h \
    recitewords.h \
    wordslib.h \
    wrongwords.h

FORMS += \
    logindialog.ui \
    mainwindow.ui \
    recitewords.ui \
    wordslib.ui \
    wrongwords.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    main.qrc
