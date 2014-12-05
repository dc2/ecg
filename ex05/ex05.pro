TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle qt
CONFIG += c++11

QMAKE_CXXFLAGS += -O0 -Wall -Wno-writable-strings

LIBS += -lGL -lGLEW -lglut -lAntTweakBar
INCLUDEPATH += include AntTweakBar/include

SOURCES += \
    src/CameraController.cpp \
    src/Ex05.cpp \
    src/MeshObj.cpp \
    src/ObjLoader.cpp

HEADERS += \
    include/CameraController.h \
    include/Ex05.h \
    include/MeshObj.h \
    include/ObjLoader.h

OTHER_FILES += \
    shader/material_and_light.frag \
    shader/material_and_light.vert

