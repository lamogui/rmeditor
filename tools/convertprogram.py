import sys
from os.path import isfile, isdir, join
from collections import OrderedDict

def writeProgram(filename, program_name , params):
    f =  open(filename, 'w');
    if (f is None):
        print("Error: unable open file " + filename);
        return;

    f.write(program_name + "\n");
    for name,value in params.items():
        f.write(name + ";" + str(value) + "\n")

def readProgram(filename):
    f = open(filename, 'r');
    if (f is None):
        print("Error: unable read open " + filename);
        sys.exit(3);

    lines = f.readlines();
    program_name = lines[0][:-1];
    params = OrderedDict();
    for i,l in enumerate(lines[1:]):

        name = None
        value = None;
        try:
            name, value = l.split(';');
        except ValueError as a:
            print("Warning: error at readin line " +  str(i+2)  + " --> "+ l);
            continue;

        value = value[:-1];
        params[name] = value;

    print("Program " + program_name + "["+ filename + "] have " + str(len(params)) + " parameters");
    return (program_name,params);



if len(sys.argv) != 3:
    print "Usage: {0} <goodprogram.txt> <fuckedprogram.txt | fuckedprogramdir>".format(sys.argv[0])
    sys.exit(2)

good_filename = sys.argv[1];

print("Parsing good program " + good_filename);
good_program_name, params = readProgram(good_filename);


files = []
if (isdir(sys.argv[2])):
    path = sys.argv[2];
    files = [join(path,f) for f in listdir(path) if isfile(join(path, f))]
elif (isfile(sys.argv[2])):
    files.append(sys.argv[2]);
else:
    print ("Error " + sys.argv[2] + " is not a file or a directory");
    print "Usage: {0} <goodprogram.txt> <fuckedprogram.txt | fuckedprogramdir>".format(sys.argv[0])
    sys.exit(1);

for f in files:
    print("Converting " + f);
    bad_program_name, bad_params = readProgram(f);

    missing_param_good = [a for a in bad_params.keys() if (a not in params.keys())];
    missing_param_bad = [a for a in params.keys() if (a not in bad_params.keys())];

    if (len(missing_param_bad) > 0):
        print("Warning: " + bad_program_name + " ["+f+"] is missing " + str(len(missing_param_bad)) + " params from good params");
        print(str(missing_param_bad));
    if (len(missing_param_good) > 0):
        print("Warning: " + good_program_name + " ["+f+"] is missing " + str(len(missing_param_good)) + " params from "+ bad_program_name + " params");
        print(str(missing_param_good));


    for name in params.keys():
        if (bad_params.has_key(name)):
            params[name] = bad_params[name];



    print("Writing " + f);
    writeProgram(f,bad_program_name,params);
