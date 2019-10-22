TEMPLATE = subdirs
SUBDIRS = \
  rtaudio \
  studio

rtaudio.subdir = rtaudio
studio.subdir = studio

studio.depends = rtaudio
