QT       += core gui network sql testlib

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 testcase

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    dbmanager.cpp \
    howlongtobeat.cpp \
    main.cpp \
    mainwindow.cpp \
    newgame.cpp \
    stats.cpp \
    updategameform.cpp \
    util.cpp \
    utiltest.cpp

# Add libcurl lib
# LIBS+=C:\Qt\6.5.1\mingw_64\lib\libcurl.a
# LIBS+=C:\Qt\6.5.1\mingw_64\lib\libcurl.dll.a

# Include libcurl include folder
# INCLUDEPATH += C:\Qt\6.5.1\mingw_64\include

HEADERS += \
    ImageUtil.h \
    dbmanager.h \
    howlongtobeat.h \
    mainwindow.h \
    newgame.h \
    stats.h \
    updategameform.h \
    util.h \
    utiltest.h

FORMS += \
    howlongtobeat.ui \
    mainwindow.ui \
    newgame.ui \
    stats.ui \
    updategameform.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    vcpkg.json

RESOURCES += \
    Resources.qrc

RC_ICONS = crono_icon.ico
