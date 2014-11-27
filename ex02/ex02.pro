TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -std=c++11 -Wall

QMAKE_LIBS += -lGL -lGLEW -lglut

SOURCES += \
    src/Ex02.cpp

HEADERS += \
    include/bunny.h \
    src/Ex02.h

OTHER_FILES += \
    shader/simple.frag \
    shader/simple.vert

