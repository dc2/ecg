TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++11

QMAKE_CXXFLAGS +=  -O0 -Wall

QMAKE_LIBS += -lGL -lGLEW -lglut
QMAKE_INCDIR += include

SOURCES += \
    src/Ex03.cpp \
    src/MeshObj.cpp \
    src/ObjLoader.cpp

HEADERS += include/Ex03.h \
    include/MeshObj.h \
    include/ObjLoader.h

OTHER_FILES += \
    shader/simple.frag \
    shader/simple.vert

