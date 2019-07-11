QT += opengl xml network widgets core

CONFIG += c++11

LIBS += -L$$PWD/lib/
release: LIBS += -lrtaudio_static -lopengl32 -lkernel32 -lOle32
debug: LIBS += -lrtaudio_staticd -lopengl32 -lkernel32 -lOle32

win32: LIBS += \
    -luuid -lksuser -ldsound
unix: LIBS += \
    -lpulse -lpulse-simple -lasound

DEPENDPATH += $$PWD/lib/

INCLUDEPATH += \
    $$PWD/include/ \
    $$PWD/dependencies/rtaudio/


win32:!win32-g++: PRE_TARGETDEPS += $$PWD/lib/rtaudio_static.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$PWD/lib/librtaudio_static.a

release:DEFINES += \
    eRELEASE

debug:DEFINES += \
    eDEBUG

win32: DEFINES += \
    eWIN32

unix: DEFINES += \
    eDEBUG


HEADERS += \
    include/camera.hpp \
    include/demotimeline.hpp \
    include/editorwidget.hpp \
    include/fast2dquad.hpp \
    include/fbo.hpp \
    include/ffmpegencoder.hpp \
    include/framework.hpp \
    include/giftexture.hpp \
    include/highlighter.hpp \
    include/keyframe.hpp \
    include/logdockwidget.hpp \
    include/logwidget.hpp \
    include/mainwindow.hpp \
    include/music.hpp \
    include/nodefile.hpp \
    include/project.hpp \
    include/renderer.hpp \
    include/renderwidget.hpp \
    include/scene.hpp \
    include/sequence.hpp \
    include/shader.hpp \
    include/shadercode.hpp \
    include/shaderminifier.hpp \
    include/texteditable.hpp \
    include/texteditor.hpp \
    include/texture.hpp \
    include/timeline.hpp \
    include/timelinedockwidget.hpp \
    include/timelinewidget.hpp \
include/types.hpp \
include/array.hpp \
include/datastream.hpp \
include/random.hpp \
include/runtime.hpp \
include/simd.hpp \
include/system.hpp \
include/threading.hpp \
include/tf4.hpp \
include/tf4fx.hpp \
include/tf4player.hpp \
    include/tunefish4music.hpp \
include/gif.h

SOURCES += \
    src/camera.cpp \
    src/demotimeline.cpp \
    src/editorwidget.cpp \
    src/fast2dquad.cpp \
    src/fbo.cpp \
    src/ffmpegencoder.cpp \
    src/giftexture.cpp \
    src/highlighter.cpp \
    src/keyframe.cpp \
    src/logdockwidget.cpp \
    src/logwidget.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/music.cpp \
    src/nodefile.cpp \
    src/project.cpp \
    src/renderer.cpp \
    src/renderwidget.cpp \
    src/scene.cpp \
    src/sequence.cpp \
    src/shader.cpp \
    src/shadercode.cpp \
    src/shaderminifier.cpp \
    src/texteditable.cpp \
    src/texteditor.cpp \
    src/texture.cpp \
    src/timeline.cpp \
    src/timelinedockwidget.cpp \
    src/timelinewidget.cpp \
src/array.cpp \
src/datastream.cpp \
src/random.cpp \
src/runtime.cpp \
src/simd.cpp \
src/threading.cpp \
src/tf4.cpp \
src/tf4fx.cpp \
src/tf4player.cpp \
    src/tunefish4music.cpp \
src/gif.c

FORMS += \
    ui/editorwidget.ui \
    ui/logdockwidget.ui \
    ui/mainwindow.ui \
    ui/timelinedockwidget.ui

RESOURCES += \
    resources/res.qrc \
    resources/qdarkstyle/style.qrc

RC_FILE += \
    resources/ressources.rc










