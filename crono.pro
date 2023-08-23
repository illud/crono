QT       += core gui network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    CustomRowCard.cpp \
    dbmanager.cpp \
    main.cpp \
    mainwindow.cpp \
    newgame.cpp \
    stats.cpp \
    util.cpp

# Add libcurl lib
LIBS+=C:\Qt\6.5.1\mingw_64\lib\libcurl.a
LIBS+=C:\Qt\6.5.1\mingw_64\lib\libcurl.dll.a

# Include libcurl include folder
INCLUDEPATH += C:\Qt\6.5.1\mingw_64\include

HEADERS += \
    CustomRowCard.h \
    ImageUtil.h \
    dbmanager.h \
    mainwindow.h \
    newgame.h \
    stats.h \
    util.h

FORMS += \
    mainwindow.ui \
    newgame.ui \
    stats.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    vcpkg.json
