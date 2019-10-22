TEMPLATE = app
QT += opengl xml network widgets core

CONFIG += c++11

LIBS += -L$$_PRO_FILE_PWD_/../lib -ldependencies
DESTDIR = $$_PRO_FILE_PWD_/../bin/

win32: LIBS += \
  -luser32 -lAdvapi32 -luuid -lksuser -ldsound -lopengl32 -lkernel32 -lOle32
unix: LIBS += \
  pulse pulse-simple asound

release:DEFINES += eRELEASE
debug:DEFINES += eDEBUG
win32: DEFINES += eWIN32

INCLUDEPATH += \
  ../include/ \
  ../dependencies/rtaudio/

HEADERS += \
  ../include/camera.hpp \
  ../include/demotimeline.hpp \
  ../include/editorwidget.hpp \
  ../include/fast2dquad.hpp \
  ../include/fbo.hpp \
  ../include/ffmpegencoder.hpp \
  ../include/framework.hpp \
  ../include/giftexture.hpp \
  ../include/highlighter.hpp \
  ../include/keyframe.hpp \
  ../include/logdockwidget.hpp \
  ../include/logwidget.hpp \
  ../include/mainwindow.hpp \
  ../include/music.hpp \
  ../include/nodefile.hpp \
  ../include/project.hpp \
  ../include/renderer.hpp \
  ../include/renderwidget.hpp \
  ../include/scene.hpp \
  ../include/sequence.hpp \
  ../include/shader.hpp \
  ../include/shadercode.hpp \
  ../include/shaderminifier.hpp \
  ../include/texteditable.hpp \
  ../include/texteditor.hpp \
  ../include/texture.hpp \
  ../include/timeline.hpp \
  ../include/timelinedockwidget.hpp \
  ../include/timelinewidget.hpp \
  ../include/types.hpp \
  ../include/array.hpp \
  ../include/datastream.hpp \
  ../include/random.hpp \
  ../include/runtime.hpp \
  ../include/simd.hpp \
  ../include/system.hpp \
  ../include/threading.hpp \
  ../include/tf4.hpp \
  ../include/tf4fx.hpp \
  ../include/tf4player.hpp \
  ../include/tunefish4music.hpp \
  ../include/gif.h

SOURCES += \
  camera.cpp \
  demotimeline.cpp \
  editorwidget.cpp \
  fast2dquad.cpp \
  fbo.cpp \
  ffmpegencoder.cpp \
  giftexture.cpp \
  highlighter.cpp \
  keyframe.cpp \
  logdockwidget.cpp \
  logwidget.cpp \
  main.cpp \
  mainwindow.cpp \
  music.cpp \
  nodefile.cpp \
  project.cpp \
  renderer.cpp \
  renderwidget.cpp \
  scene.cpp \
  sequence.cpp \
  shader.cpp \
  shadercode.cpp \
  shaderminifier.cpp \
  texteditable.cpp \
  texteditor.cpp \
  texture.cpp \
  timeline.cpp \
  timelinedockwidget.cpp \
  timelinewidget.cpp \
  array.cpp \
  datastream.cpp \
  random.cpp \
  runtime.cpp \
  simd.cpp \
  threading.cpp \
  tf4.cpp \
  tf4fx.cpp \
  tf4player.cpp \
  tunefish4music.cpp \
  gif.c

FORMS += \
  ../ui/editorwidget.ui \
  ../ui/logdockwidget.ui \
  ../ui/mainwindow.ui \
  ../ui/timelinedockwidget.ui

RESOURCES += \
  ../resources/res.qrc \
  ../resources/qdarkstyle/style.qrc

RC_FILE += \
  ../resources/ressources.rc










