TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++11

SOURCES += main.cpp \
    board.cpp \
    position.cpp

HEADERS += \
    bitboard.h \
    board.h \
    types.h \
    position.h

