TEMPLATE = subdirs
SUBDIRS = \
  submodules \
  enigma \
  tunefish \
  studio

submodules.subdir = submodules #ugly but no choices with submodules if we don't want modify submodule repo
enigma.subdir = enigma
tunefish.subdir = tunefish
studio.subdir = studio

tunefish.depends = enigma
studio.depends = submodules tunefish










