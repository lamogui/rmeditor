TEMPLATE = app
QT += opengl xml network widgets core
CONFIG += c++11

win32: LIBS += \
  uuid ksuser dsound opengl32 kernel32 Ole32
unix: LIBS += \
  pulse pulse-simple asound

release:DEFINES += eRELEASE
debug:DEFINES += eDEBUG
win32: DEFINES += eWIN32

INCLUDEPATH += ../submodules/rtaudio
HEADERS += \
  4klangmusic.hpp \
  camera.hpp \
  demotimeline.hpp \
  fast2dquad.hpp \
  ffmpegencoder.hpp \
  gif.h \
  giftexture.hpp \
  highlighter.hpp \
  jassert.hpp \
  keyframe.hpp \
  logdockwidget.hpp \
  logmanager.hpp \
  logwidget.hpp \
  mainwindow.hpp \
  mediafile.hpp \
  mediafileseditorwidget.hpp \
  music.hpp \
  oggvorbismusic.hpp \
  project.hpp \
  quadfragmentshader.hpp \
  render.hpp \
  renderer.hpp \
  renderfunctionscache.hpp \
  renderwidget.hpp \
  scene.hpp \
  sequence.hpp \
  shadercode.hpp \
  shaderminifier.hpp \
  texteditable.hpp \
  texteditor.hpp \
  timeline.hpp \
  timelinedockwidget.hpp \
  timelinetrack.hpp \
  timelinewidget.hpp \
  tunefish4music.hpp \
  undocommands.hpp \
  xmlsavedobject.hpp

SOURCES += \
  camera.cpp \
  demotimeline.cpp \
  fast2dquad.cpp \
  ffmpegencoder.cpp \
  gif.c \
  giftexture.cpp \
  highlighter.cpp \
  keyframe.cpp \
  logdockwidget.cpp \
  logmanager.cpp \
  logwidget.cpp \
  main.cpp \
  mainwindow.cpp \
  mediafile.cpp \
  mediafileseditorwidget.cpp \
  music.cpp \
  oggvorbismusic.cpp \
  project.cpp \
  quadfragmentshader.cpp \
  render.cpp \
  renderer.cpp \
  renderfunctionscache.cpp \
  renderwidget.cpp \
  scene.cpp \
  sequence.cpp \
  shadercode.cpp \
  shaderminifier.cpp \
  texteditable.cpp \
  texteditor.cpp \
  timeline.cpp \
  timelinedockwidget.cpp \
  timelinetrack.cpp \
  timelinewidget.cpp \
  tunefish4music.cpp \
  undocommands.cpp \
  xmlsavedobject.cpp

FORMS += \
  editorwidget.ui \
  logdockwidget.ui \
  mainwindow.ui \
  timelinedockwidget.ui

RESOURCES += ../icons/resources.qrc
RC_FILE += resources.rc
