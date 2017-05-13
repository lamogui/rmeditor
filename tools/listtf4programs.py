#!/usr/bin/env python
import sys
from os import listdir
from os.path import isfile, join

if len(sys.argv) < 2:
    print("Usage:" + sys.argv[0] + " <program_dir>")
    sys.exit(2)

mypath = sys.argv[1];
programfiles = [f for f in listdir(mypath) if isfile(join(mypath, f))];
programs = {};


for programfilename in programfiles:
    programfile = open(join(mypath,programfilename),'r');
    if programfile is None:
        print ("Error could not find: " + programfilename );
        continue;

    try:
        params = programfile.readlines();
        print (programfilename + " ---> "+ params[0][:-1]);
        programs[params[0][:-1]] = programfilename;

    except IndexError as e:
        print("Error file: " + programfilename + " is invalid (" + str(e) + ")");


print ("\n Inverted list");
for name,filename in sorted(programs.items()):
    print (name + " ---> "+ filename);
