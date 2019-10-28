TEMPLATE = subdirs
SUBDIRS = \
  rtaudio \
  tunefish \
  studio

win32:SUBDIRS += WaveSabre

rtaudio.subdir = rtaudio
tunefish.subdir = tunefish
studio.subdir = studio
win32:WaveSabre.subdir = WaveSabre

studio.depends = rtaudio tunefish
win32:studio.depends += WaveSabre
