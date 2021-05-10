@echo off
if not exist build\win64 (
    mkdir build\win64
)
cd build\win64

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
cmake ..\..\ -G "Visual Studio 16 2019" -A x64 -DCMAKE_INSTALL_PREFIX="Sdk"
cmake --build .
cd ..\..\
pause