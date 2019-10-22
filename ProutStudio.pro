TEMPLATE = subdirs
SUBDIRS = \
  rtaudio \
  tunefish \
  studio

rtaudio.subdir = rtaudio
tunefish.subdir = tunefish
studio.subdir = studio

studio.depends = rtaudio tunefish
