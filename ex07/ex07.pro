TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle qt
CONFIG += c++11

QMAKE_CXXFLAGS += -O0 -Wall -Wno-writable-strings

LIBS += -lGL -lGLEW -lglut -lAntTweakBar `sdl2-config --libs` -lSDL2_image -lassimp 
INCLUDEPATH += include Ant

SOURCES += \
    src/CameraController.cpp \
    src/Ex07.cpp \
    src/MeshObj.cpp \
    src/ObjLoader.cpp \
    lib/Shader.cpp \
    lib/ShaderProgram.cpp

HEADERS += \
    include/CameraController.h \
    include/Ex07.h \
    include/MeshObj.h \
    include/ObjLoader.h \
    lib/Shader.h \
    lib/ShaderProgram.h

OTHER_FILES += \
    shader/material_and_light.frag \
    shader/material_and_light.vert \
    shader/simple.frag \
    shader/simple.vert

DISTFILES += \
    shader/texture.frag \
    shader/texture.vert

