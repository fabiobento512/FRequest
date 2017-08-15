#-------------------------------------------------
#
# Project created by QtCreator 2017-03-17T15:24:31
#
#-------------------------------------------------

QT       += core gui network
CONFIG   += c++14
QMAKE_CXXFLAGS += -Wall
QMAKE_CXXFLAGS += -Wextra

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

include(../CommonUtils/CommonUtils.pri)
include(../CommonLibs/CommonLibs.pri)

TARGET = FRequest
TEMPLATE = app

macx {
ICON = frequest_icon.icns # mac os icon
}

win32 {
    RC_FILE = icon_resource.rc #for windows explorer icon
}

SOURCES += main.cpp\
        mainwindow.cpp \
    utilfrequest.cpp \
    about.cpp \
    preferences.cpp \
    configfilefrequest.cpp \
    HttpRequests/deletehttprequest.cpp \
    HttpRequests/httprequest.cpp \
    HttpRequests/posthttprequest.cpp \
    HttpRequests/puthttprequest.cpp \
    HttpRequests/gethttprequest.cpp \
    HttpRequests/patchhttprequest.cpp \
    HttpRequests/headhttprequest.cpp \
    HttpRequests/tracehttprequest.cpp \
    HttpRequests/optionshttprequest.cpp \
    Widgets/frequesttreewidgetitem.cpp


HEADERS  += mainwindow.h \
    utilglobalvars.h \
    utilfrequest.h \
    about.h \
    preferences.h \
    configfilefrequest.h \
    HttpRequests/deletehttprequest.h \
    HttpRequests/httprequest.h \
    HttpRequests/posthttprequest.h \
    HttpRequests/puthttprequest.h \
    HttpRequests/gethttprequest.h \
    HttpRequests/patchhttprequest.h \
    HttpRequests/headhttprequest.h \
    HttpRequests/tracehttprequest.h \
    HttpRequests/optionshttprequest.h \
    Widgets/frequesttreewidgetitem.h


FORMS    += mainwindow.ui \
    about.ui \
    preferences.ui

RESOURCES += \
    resources.qrc
