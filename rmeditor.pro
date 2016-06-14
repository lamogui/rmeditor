QT += opengl xml network widgets core

CONFIG += c++11

LIBS += -L$$PWD/lib/
Release: LIBS += -lrtaudio_static -ltunefish4
Debug: LIBS += -lrtaudio_staticd -ltunefish4d

win32: LIBS += \
    -luuid -lksuser -ldsound
unix: LIBS += \
    -lpulse -lpulse-simple -lasound

DEPENDPATH += $$PWD/lib/

INCLUDEPATH += \
    $$PWD/include/ \
    $$PWD/dependencies/tunefish4/include/ \
    $$PWD/dependencies/rtaudio/


win32:!win32-g++: PRE_TARGETDEPS += $$PWD/lib/rtaudio_static.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$PWD/lib/librtaudio_static.a

Release:DEFINES += \
    eRELEASE

Debug:DEFINES += \
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
    include/highlighter.hpp \
    include/keyframe.hpp \
    include/logdockwidget.hpp \
    include/logwidget.hpp \
    include/mainwindow.hpp \
    include/music.hpp \
    include/nodefile.hpp \
    include/player.hpp \
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
    include/tunefish4music.hpp

SOURCES += \
    src/camera.cpp \
    src/demotimeline.cpp \
    src/editorwidget.cpp \
    src/fast2dquad.cpp \
    src/fbo.cpp \
    src/ffmpegencoder.cpp \
    src/highlighter.cpp \
    src/keyframe.cpp \
    src/logdockwidget.cpp \
    src/logwidget.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/music.cpp \
    src/nodefile.cpp \
    src/player.cpp \
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
    src/tunefish4music.cpp

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










