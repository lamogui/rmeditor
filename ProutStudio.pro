TEMPLATE = subdirs
SUBDIRS = \
  dependencies \
  src

dependencies.subdir = dependencies 
src.subdir = src

src.depends = dependencies
