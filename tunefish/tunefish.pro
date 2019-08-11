TEMPLATE = lib
CONFIG += staticlib stl_off rtti_off

INCLUDEPATH += ../enigma

release:DEFINES += eRELEASE
debug:DEFINES += eDEBUG
win32: DEFINES += eWIN32

HEADERS += \
  tf4.hpp \
  tf4dx.hpp \
  tf4fx.hpp \
  tf4player.hpp

SOURCES += \
  tf4.cpp \
  tf4dx.cpp \
  tf4fx.cpp \
  tf4player.cpp
