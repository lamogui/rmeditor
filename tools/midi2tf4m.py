#!/usr/bin/env python
import midi
import sys
from struct import unpack, pack
from os import listdir
from os.path import isfile, isdir, join


ROW_PER_BEAT = 96;
TF_PARAM_COUNT = 112;
DEFAULT_TEMPO = 140;


def writeU16(f,n):
    assert n < 65536,"U16 overflow !"
    assert n >= 0,"Signed unsigned"
    f.write(pack('<H',n));

def writeU8(f,n):
    assert n < 256,"U8 overflow !"
    assert n >= 0,"Signed unsigned"
    f.write(pack('<B',n));


if len(sys.argv) < 3:
    print "Usage: {0} <midifile> <tf4mfile> <program_track0.txt> <program_track1.txt> ...".format(sys.argv[0])
    sys.exit(2)

def AssociatedProgramFilename(name, path):
    files = [f for f in listdir(path) if isfile(join(path, f))];
    for programfilename in files:
        programfile = open(join(path,programfilename),'r');
        if programfile is None:
            print ("Error could not open: " + programfilename );
            continue;
        try:
            params = programfile.readlines();
            if  params[0][:-1].strip() == name.strip():
                return join(path,programfilename);

        except IndexError as e:
            print("Error file: " + programfilename + " is invalid (" + str(e) + ")");
    return None;

midifilename = sys.argv[1]
tf4mfilename = sys.argv[2]
midifile = midi.read_midifile(midifilename)


reso_bytes = unpack('bB',pack('>H',midifile.resolution));
tempo = DEFAULT_TEMPO;
rows_per_min = tempo * ROW_PER_BEAT;
rows_per_sec = float(rows_per_min) / 60.0;
secs_per_row = 1.0 / rows_per_sec;

tempo_set_count = 0;

#Normalement avec le format MIDI 1 il est suppose avoir une track 0 qui contient
#les infos sur le tempo
print("Resampling the MIDI from resolution " + str(midifile.resolution) + " to resolution " + str(ROW_PER_BEAT) + " (" + str(int(100.0*float(ROW_PER_BEAT)/midifile.resolution)) + "%)");

midifile.make_ticks_abs();
precision_losed = 0;
for i,track in enumerate(midifile):
    track_have_notes = False;
    for e,event in enumerate(track):
        new_tick = int(round(float(event.tick) * float(ROW_PER_BEAT) / float(midifile.resolution)));
        computed_tick = int(new_tick*midifile.resolution / ROW_PER_BEAT);
        if (event.tick != computed_tick):
            #print ("Warning: losing resolution at event " + str(e) + " (track " + str(i) + ") original tick: " + str(event.tick) + " new tick: " + str(new_tick) + " (" + str(computed_tick) + ")");
            precision_losed = precision_losed + 1;

        event.tick = new_tick;

        if (event.name == 'Set Tempo'):
            if (event.tick > 0):
                print("Error: the tempo is in the middle of the track is not supported");
                sys.exit(7);
            tempo = bpm;
            tempo_set_count = tempo_set_count + 1;
        elif (event.name == 'Note On'):
            track_have_notes = True;

    if (track_have_notes==False):
        midifile.remove(track);
        print("Removing track " + str(i) + " (track doesn't contains any notes)");


if (tempo_set_count > 1):
    print("Error: the tempo is set multiple times this is not supported");
    sys.exit(8);

if (precision_losed > 0):
    nb_event = 0;
    for t in midifile:
        nb_event = nb_event + len(t);
    print("Warning: precision losed of " + str(precision_losed) + "/" + str(nb_event) + " events (" + str(int(100.0*precision_losed/nb_event)) + "%)" );


midifile.make_ticks_rel();

#print(repr(midifile))



print("Resolution bytes: " + str(reso_bytes));
print ("Format: " + str(midifile.format));
print ("Resolution: " + str(midifile.resolution));
print ("Tick Relative: " + str(midifile.tick_relative));
print ("Tracks: " + str(len(midifile)))
print ("Tempo: " + str(tempo) + "bpm");
print ("ROW_PER_BEAT: " + str(ROW_PER_BEAT));
print ("rows_per_min: " + str(rows_per_min));
print ("rows_per_sec: " + str(rows_per_sec));
print ("secs_per_row: " + str(secs_per_row));



if (midifile.format != 1):
    print "Error midifile format is not format 1 please use midifile format 1 (separated tracks)";
    sys.exit(3);

programfiles = [];

if (len(sys.argv) == 4 and isdir(sys.argv[3])):
    path = sys.argv[3];
    print("Try to detect automatically associated program using directory " + path);
    for i,track in enumerate(midifile):
        filename = None;
        for event in track:
            if (event.name == 'Track Name'):
                filename = AssociatedProgramFilename(event.text,path);
                if (filename is None):
                    print ("Warning: track" + str(i) + " have 'Track Name' event '" + event.text + "' not associated with program");
                    continue;
                else:
                    print("Associate track " +  str(i) + " [Track Name " + event.text + "] with program " + filename);
                    break;
        if (filename is None):
            print ("Error: could not find associated program for Track " + str(i) + " abort")
            sys.exit(15);
        else:
            programfiles.append(filename);

elif len(sys.argv) != 3 + len(midifile):
    print ("Error: you need to specify " + str(len(midifile)) + " tunefish programs or a tunefish4 program directoy");
    print ("Usage: {0} <midifile> <tf4mfile> <program_track0.txt> <program_track1.txt> ...".format(sys.argv[0]))
    sys.exit(4)
else:
    programfiles = sys.argv[3:];


tf4mfile = open(tf4mfilename,'wb');
writeU16(tf4mfile,len(midifile)); #nb of instruments
writeU16(tf4mfile,tempo);         #tempo
writeU16(tf4mfile,ROW_PER_BEAT);  #ROW_PER_BEAT


event_count = [];
event_time = [];
event_notes = [];
event_velocity = [];

for i,track in enumerate(midifile):
    unused = 0;
    event_count.append(0);
    event_time.append([]);
    event_notes.append([]);
    event_velocity.append([]);
    event_tick_stack = 0;
    unused_event_list = [];
    for event in track:
        if event.name == 'Note On' or event.name == 'Note Off':
            event_count[i] = event_count[i]+1;
            event_time[i].append(event.tick+event_tick_stack);
            event_notes[i].append(event.pitch);
            if (event.name == 'Note On'):
                event_velocity[i].append(event.velocity);
            else:
                event_velocity[i].append(0);
            event_tick_stack = 0;

        elif event.name == 'Track Name' or event.name == 'End of Track':
            event_tick_stack = event_tick_stack + event.tick; 
            pass;
        else:
            
            unused = unused + 1;
            event_tick_stack = event_tick_stack + event.tick; 
            if (event.name in unused_event_list):
              pass;
            else:
              print("Track " + str(i) + ": Unused event '" + event.name + "'");
              unused_event_list.append(event.name);
            

    print("Track " + str(i) + ": " + str(event_count[i]) + " events (probably " + str(event_count[i]/2) + " notes)");
    if (unused != 0):
        print ("Track " + str(i) + ": Warning " + str(unused) + " event unused");

for c in event_count:
    writeU16(tf4mfile,c); #nb of event by instruments

tf4mfile.write("INST"); #tag inst


for programfilename in programfiles:
    programfile = open(programfilename,'r');
    if programfile is None:
        print ("Error could not find: " + programfilename + " abort");
        sys.exit(5);

    try:
        params = programfile.readlines();
        print ("Reading param of instrument " + params[0][:-1]);
        for i in range(TF_PARAM_COUNT):
            name, value = params[i+1].split(';');
            value = int(float(value)*100);
            #print ("Write Param " + name + " with value of " + str(value));
            writeU8(tf4mfile,value); #instrument param

    except IndexError as e:
        print("Error file: " + programfilename + " is invalid (" + str(e) + ")");
        sys.exit(6);

tf4mfile.write("SONG"); #tag inst
for i,track in enumerate(midifile):
    for t in event_time[i]:
        writeU16(tf4mfile,t);
    for n in event_notes[i]:
        writeU8(tf4mfile,n);
    for v in event_velocity[i]:
        writeU8(tf4mfile,v);

tf4mfile.write("ENDS"); #tag ends
tf4mfile.close();

print("(probably) Success ! Written in " + tf4mfilename);
