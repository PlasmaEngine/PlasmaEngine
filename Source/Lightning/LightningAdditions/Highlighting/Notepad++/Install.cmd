@echo off
echo Make sure you close out any instances of Notepad++.
pause

Rem - We install a file that adds keywords to visual studio 'usertype.dat'
Rem - We also then create a file extension type for the Lightning file, and associate it with VS
cd Files
copy /Y "Lightning.xml" "%APPDATA%\Notepad++\"
ftype lightning="notepad++.exe" "%%1"
assoc .z=lightning

Rem - We're done!
cls
echo Notepad++ syntax highlighter installed!
pause
