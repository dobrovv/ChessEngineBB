TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++11

SOURCES += main.cpp \
    bitboard.cpp \
    board.cpp \
    movegen.cpp \
    position.cpp

HEADERS += \
    bitboard.h \
    board.h \
    types.h \
    movegen.h \
    position.h

