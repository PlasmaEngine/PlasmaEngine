@echo off
if not exist build\ninja (
    mkdir build\ninja
)
cd build\ninja

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
cmake ..\..\ -G "Ninja" -DCMAKE_BUILD_TYPE:STRING="Release" -DCMAKE_INSTALL_PREFIX="Sdk"
cmake --build . --target install --config Release
cd ..\..\
pause