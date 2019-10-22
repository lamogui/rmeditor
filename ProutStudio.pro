TEMPLATE = subdirs
SUBDIRS = \
  dependencies \
  studio

dependencies.subdir = dependencies 
studio.subdir = studio

studio.depends = dependencies
