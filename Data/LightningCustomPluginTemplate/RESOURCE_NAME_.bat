@ECHO OFF
REM If we launch Plasma from Visual Studio, and then run another 'sln' from Plasma it will implicitly
REM pass through all environmental variables, which we may want to clear or change
PUSHD "%~dp0"
SET VisualStudioVersion=
"RESOURCE_NAME_.sln"
