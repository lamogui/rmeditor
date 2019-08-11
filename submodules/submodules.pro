TEMPLATE = lib
#I allow me a warn_off here since it's submodules and we want avoid modify source code...
CONFIG += staticlib warn_off stl_off rtti_off

#ogg
INCLUDEPATH += ogg/include
HEADERS += \
  ogg/include/ogg/os_types.h \
  ogg/include/ogg/ogg.h
SOURCES += \
  ogg/src/bitwise.c \
  ogg/src/framing.c

#vorbis
INCLUDEPATH += vorbis/include
HEADERS += \
  vorbis/include/vorbis/codec.h \
  vorbis/include/vorbis/vorbisenc.h \
  vorbis/include/vorbis/vorbisfile.h \
  vorbis/lib/envelope.h \
  vorbis/lib/lpc.h \
  vorbis/lib/lsp.h \
  vorbis/lib/codebook.h \
  vorbis/lib/misc.h \
  vorbis/lib/psy.h \
  vorbis/lib/masking.h \
  vorbis/lib/os.h \
  vorbis/lib/mdct.h \
  vorbis/lib/smallft.h \
  vorbis/lib/highlevel.h \
  vorbis/lib/registry.h \
  vorbis/lib/scales.h \
  vorbis/lib/window.h \
  vorbis/lib/lookup.h \
  vorbis/lib/lookup_data.h \
  vorbis/lib/codec_internal.h \
  vorbis/lib/backends.h \
  vorbis/lib/bitrate.h
SOURCES += \
  vorbis/lib/mdct.c \
  vorbis/lib/smallft.c \
  vorbis/lib/block.c \
  vorbis/lib/envelope.c \
  vorbis/lib/window.c \
  vorbis/lib/lsp.c \
  vorbis/lib/lpc.c \
  vorbis/lib/analysis.c \
  vorbis/lib/synthesis.c \
  vorbis/lib/psy.c \
  vorbis/lib/info.c \
  vorbis/lib/floor1.c \
  vorbis/lib/floor0.c \
  vorbis/lib/res0.c \
  vorbis/lib/mapping0.c \
  vorbis/lib/registry.c \
  vorbis/lib/codebook.c \
  vorbis/lib/sharedbook.c \
  vorbis/lib/lookup.c \
  vorbis/lib/bitrate.c \
  vorbis/lib/vorbisfile.c \
  vorbis/lib/vorbisenc.c

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

