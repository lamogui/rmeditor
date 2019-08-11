TEMPLATE = lib
CONFIG += debug_and_release staticlib stl_off rtti_off

release:DEFINES += eRELEASE
debug:DEFINES += eDEBUG
win32: DEFINES += eWIN32

HEADERS += \
  array.hpp \
  datastream.hpp \
  random.hpp \
  runtime.hpp \
  simd.hpp \
  system.hpp \
  threading.hpp \
  types.hpp

SOURCES += \
  array.cpp \
  datastream.cpp \
  runtime.cpp \
  simd.cpp \
  threading.cpp
