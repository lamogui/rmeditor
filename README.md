# rmeditor

[![Build Status](https://travis-ci.org/lamogui/rmeditor.svg?branch=master)](https://travis-ci.org/lamogui/rmeditor) [![Build status](https://ci.appveyor.com/api/projects/status/y0xccwo4xaujtuus?svg=true)](https://ci.appveyor.com/project/lamogui/rmeditor)

![screenshot](http://s31.postimg.org/jcs4o844r/screenshot.png)


## History
This demotool is initialy a merge of:
 - [Visual Live System by xt95](https://github.com/XT95/VisualLiveSystem)
 - [Tunefish4 by paynebc](https://github.com/paynebc/tunefish)

Some code came from [Enigma Studio 4 by Brain Control]
(https://github.com/enigmastudio/Enigma-Studio-4) because Tunefish4 code crashed
on linux.


## Requirements
 * Qt5 (latest version you can found) 
 * CMake (cmake-gui recommanded)


## How to compile

 * Clone the repository 
```
git clone https://github.com/lamogui/rmeditor.git --recursive
```
 * Init all submodules (dependencies) (if your forgot the --recursive while cloning)
```
git submodule update --init --recursive
```
 * Run cmake-gui with the rmeditor/CMakeLists.txt
 * Uncheck *BUILD_TESTING* (because rtAudio test program fail to link libraries)

#### Windows
 * Select _add entry_ add _CMAKE_PREFIX_PATH_ set the variable type to PATH and set your Qt\5.*\msvc_* build directory
 * Check *AUDIO_WINDOWS_WASAPI* and/or *AUDIO_WINDOWS_DS*

#### Linux
 * Check *AUDIO_LINUX_PULSE*

All dependencies should now be present inside the *lib* directory (the 
dependencies that end with d are the debug one).
All rmeditor.exe and the Qt dll should be copied to /bin

Note: if have compiled a static version of Qt you can check _RMEDITOR_USE_QT_STATIC_ (and _MSVC_STATIC_ on windows) to build standalone executable version of the editor
 

