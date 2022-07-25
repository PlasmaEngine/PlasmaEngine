@echo off
if not exist build\win64 (
    mkdir build\VisualStudio2022_win64
)
cd build\VisualStudio2022_win64

setlocal ENABLEEXTENSIONS

:: try to use vswhere.exe to get VS install dir
if EXIST "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (

	FOR /F "tokens=* USEBACKQ" %%F IN (`call "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath`) DO (
	SET "product_dir=%%F")
)

if "%product_dir%" NEQ "" goto :EndLoop

:: Finding VS install by iterating over possible versions in the registry
set VSVer[0]=17.0
set VSVer[1]=16.0
set VSVer[2]=15.0
set VSVer[3]=14.0
set VSVer[4]=13.0
set VSVer[5]=12.0
set VSVer[6]=11.0
set "index=0"

:VerLoop
if not defined VSVer[%index%] goto :EndLoop

:: 32-bit system:
::call set KEY_NAME="HKLM\SOFTWARE\Microsoft\VisualStudio\%%VSVer[%index%]%%\Setup\VS"
:: 64-bit system:
call set KEY_NAME=HKLM\SOFTWARE\WOW6432Node\Microsoft\VisualStudio\%%VSVer[%index%]%%\Setup\VS
set VALUE_NAME=ProductDir

call echo VS %%VSVer[%index%]%%:

FOR /F "usebackq tokens=2,* skip=2" %%L IN ( `reg query %KEY_NAME% /v %VALUE_NAME%` ) DO ( 
SET product_dir=%%M)

set /a "index+=1"
if "%product_dir%" NEQ "" goto :EndLoop
GOTO :VerLoop


:EndLoop
if defined product_dir (
echo Visual Studio Product Dir: %product_dir%
) else (
echo %KEY_NAME%\%VALUE_NAME% not found.
pause
exit
)

call "%product_dir%\VC\Auxiliary\Build\vcvars64.bat"
cmake ..\..\ -G "Visual Studio 17 2022" -A x64 -DCMAKE_INSTALL_PREFIX="Sdk"
cmake --build .
cd ..\..\
pause