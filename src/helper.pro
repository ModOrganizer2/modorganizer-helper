#-------------------------------------------------
#
# Project created by QtCreator 2011-12-23T17:43:42
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET   = helper
CONFIG   += console
CONFIG   -= app_bundle

DEFINES += UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS

TEMPLATE = app

QMAKE_LFLAGS += /MANIFESTUAC:\"level=\'requireAdministrator\' uiAccess=\'false\'\"

LIBS += -ladvapi32

CONFIG(debug, debug|release) {
	SRCDIR = $$OUT_PWD/debug
	DSTDIR = $$PWD/../../outputd
} else {
	SRCDIR = $$OUT_PWD/release
	DSTDIR = $$PWD/../../output
}

SRCDIR ~= s,/,$$QMAKE_DIR_SEP,g
DSTDIR ~= s,/,$$QMAKE_DIR_SEP,g

QMAKE_POST_LINK += mkdir -p $$quote($$DSTDIR) $$escape_expand(\\n)
QMAKE_POST_LINK += xcopy /y /I $$quote($$SRCDIR\\helper*.exe) $$quote($$DSTDIR) $$escape_expand(\\n)

SOURCES += main.cpp \
    privileges.cpp

HEADERS += \
    privileges.h
