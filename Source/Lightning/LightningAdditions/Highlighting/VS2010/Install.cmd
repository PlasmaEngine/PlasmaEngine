@echo off
echo Make sure you close out any instances of Visual Studio 2010.
pause

Rem - We install a file that adds keywords to visual studio 'usertype.dat'
Rem - We also then create a file extension type for the Lightning file, and associate it with VS
cd Files
regedit.exe /s VS2010.reg
copy /Y "usertype.dat" "%VS100COMNTOOLS%..\IDE\"
ftype lightning="devenv.exe" "%%1"
assoc .z=lightning

Rem - We're done!
cls
echo Visual studio syntax highlighter installed!
pause
