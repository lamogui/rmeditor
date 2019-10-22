TEMPLATE = app
QT += opengl xml network widgets core

CONFIG += c++11

LIBS += -L$$_PRO_FILE_PWD_/../lib -lrtaudio
DESTDIR = $$_PRO_FILE_PWD_/../bin/

win32: LIBS += \
  -luser32 -lAdvapi32 -luuid -lksuser -ldsound -lopengl32 -lkernel32 -lOle32
unix: LIBS += \
  -lpulse -lpulse-simple -lasound

release:DEFINES += eRELEASE
debug:DEFINES += eDEBUG
win32: DEFINES += eWIN32

INCLUDEPATH += \
  ../rtaudio/

HEADERS += \
  camera.hpp \
  demotimeline.hpp \
  editorwidget.hpp \
  fast2dquad.hpp \
  fbo.hpp \
  ffmpegencoder.hpp \
  framework.hpp \
  gif.hpp \
  giftexture.hpp \
  highlighter.hpp \
  keyframe.hpp \
  logdockwidget.hpp \
  logwidget.hpp \
  mainwindow.hpp \
  music.hpp \
  nodefile.hpp \
  project.hpp \
  renderer.hpp \
  renderwidget.hpp \
  scene.hpp \
  sequence.hpp \
  shader.hpp \
  shadercode.hpp \
  shaderminifier.hpp \
  texteditable.hpp \
  texteditor.hpp \
  texture.hpp \
  timeline.hpp \
  timelinedockwidget.hpp \
  timelinewidget.hpp \
  types.hpp \
  array.hpp \
  datastream.hpp \
  random.hpp \
  runtime.hpp \
  simd.hpp \
  system.hpp \
  tf4.hpp \
  tf4fx.hpp \
  tf4player.hpp \
  tunefish4music.hpp

SOURCES += \
  camera.cpp \
  demotimeline.cpp \
  editorwidget.cpp \
  fast2dquad.cpp \
  fbo.cpp \
  ffmpegencoder.cpp \
  giftexture.cpp \
  gif.cpp \
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
  tf4.cpp \
  tf4fx.cpp \
  tf4player.cpp \
  tunefish4music.cpp

FORMS += \
  editorwidget.ui \
  logdockwidget.ui \
  mainwindow.ui \
  timelinedockwidget.ui

RESOURCES += \
  ../icons/resources.qrc

RC_FILE += \
  resources.rc










