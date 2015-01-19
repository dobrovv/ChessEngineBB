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


# remove possible other optimization flags
QMAKE_CXXFLAGS_RELEASE -= -O
QMAKE_CXXFLAGS_RELEASE -= -O1
QMAKE_CXXFLAGS_RELEASE -= -O2

# add the desired -O3 if not present
QMAKE_CXXFLAGS_RELEASE += -Ofast
