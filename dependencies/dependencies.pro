TEMPLATE = lib
#I allow me a warn_off here since it's submodules and we want avoid modify source code...
CONFIG += staticlib warn_off stl_off rtti_off

#RtAudio
win32:DEFINES += __WINDOWS_DS__ __WINDOWS_WASAPI__ __WINDOWS_ASIO__
unix:DEFINES += __LINUX_PULSE__
INCLUDEPATH += rtaudio/include
HEADERS += \
  rtaudio/RtAudio.h

SOURCES+= \
  rtaudio/RtAudio.cpp
win32:SOURCES += \
  rtaudio/include/asio.cpp \
  rtaudio/include/asiodrivers.cpp \
  rtaudio/include/asiolist.cpp \
  rtaudio/include/iasiothiscallresolver.cpp

