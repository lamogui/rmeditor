REM Just an example to quickly compile Qt static and release
REM taken from https://retifrav.github.io/blog/2018/02/17/build-qt-statically/
REM install in default path clang LLVM
REM use the dev msvc console to make it works
REM you need python perl and jom https://wiki.qt.io/Jom
configure.bat -static -static-runtime -no-pch -prefix "D:\Qt\win64-msvc-static" -skip webengine -nomake tools -nomake tests -nomake examples -platform win32-msvc -debug-and-release -opengl desktop -static-runtime -verbose
jom -j8 install