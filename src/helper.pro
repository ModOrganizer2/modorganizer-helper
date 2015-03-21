#-------------------------------------------------
#
# Project created by QtCreator 2011-12-23T17:43:42
#
#-------------------------------------------------

TEMPLATE = app

TARGET   = helper

QT       += core
QT       -= gui

CONFIG   += console
CONFIG   -= app_bundle
CONFIG += embed_manifest_exe


!include(../LocalPaths.pri) {
  message("paths to required libraries need to be set up in LocalPaths.pri")
}

SOURCES += \
    main.cpp \
    privileges.cpp

HEADERS += \
    privileges.h

DEFINES += \
    UNICODE \
    _UNICODE \
    _CRT_SECURE_NO_WARNINGS

#QMAKE_LFLAGS += /MANIFESTUAC:"level=\'requireAdministrator\'uiAccess=\'false\'"
QMAKE_LFLAGS += /MANIFESTUAC:\"level=\'requireAdministrator\' uiAccess=\'false\'\"

LIBS += -ladvapi32

QMAKE_POST_LINK += xcopy /y /I $$quote($$SRCDIR\\helper*.exe) $$quote($$DSTDIR) $$escape_expand(\\n)
