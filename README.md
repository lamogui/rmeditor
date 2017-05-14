# rmeditor

[![Build Status](https://travis-ci.org/lamogui/rmeditor.svg?branch=master)](https://travis-ci.org/lamogui/rmeditor)

![screenshot](http://s31.postimg.org/jcs4o844r/screenshot.png)


## History
This demotool is initialy a merge of:
 - [Visual Live System by xt95](https://github.com/XT95/VisualLiveSystem)
 - [Tunefish4 by paynebc](https://github.com/paynebc/tunefish)

Some code came from [Enigma Studio 4 by Brain Control]
(https://github.com/enigmastudio/Enigma-Studio-4) because Tunefish4 code crashed
on linux.


## Requirements
 * Qt5 (latest version on linux)
 * QtCreator 
 * CMake (cmake-gui recommanded) used to build dependencies


## How to compile

Clone the repository

```
git clone https://github.com/lamogui/rmeditor.git
```

Init all submodules (dependencies)

```
git submodule update --init --recursive
```

Compile all dependencies using CMake (QtCreator can be used as IDE for CMake)

```
mkdir build-dependencies
cd build-dependencies
cmake-gui ../dependencies
```

#### Windows
Check *AUDIO_WINDOWS_WASAPI* and/or *AUDIO_WINDOWS_DS*

#### Linux
Check *AUDIO_LINUX_PULSE*


#### Debug/Release
In order to run the program in Debug and Release Mode you need to complie the 
dependencies twice (use *CMAKE_BUILD_TYPE* with *Debug*, *Release*, or *MinSizeRel*)

All dependencies should now be present inside the *lib* directory (the 
dependencies that end with d are the debug one).

You can now run QtCreator with **rmeditor.pro**
 

