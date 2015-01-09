TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle qt
CONFIG += c++11

DEFINES += __GXX_EXPERIMENTAL_CXX0X__

QMAKE_CXXFLAGS += -O0 -Wall -Wno-writable-strings

LIBS += -lGL -lGLEW -lglut -lAntTweakBar
INCLUDEPATH += include AntTweakBar/include

HEADERS += \
    include/CameraController.h \
    include/Ex04.h \
    include/MeshObj.h \
    include/ObjLoader.h

SOURCES += \
    src/CameraController.cpp \
    src/Ex04.cpp \
    src/MeshObj.cpp \
    src/ObjLoader.cpp

OTHER_FILES += \
    shader/ndc.frag \
    shader/ndc.vert


