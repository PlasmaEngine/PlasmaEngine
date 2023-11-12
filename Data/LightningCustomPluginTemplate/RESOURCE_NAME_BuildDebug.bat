@ECHO OFF
REM Used to automatically build the plugin in Debug
PUSHD "%~dp0"
SET VisualStudioVersion=
REM "%VS140COMNTOOLS%/../IDE/devenv" "RESOURCE_NAME_.sln" /build Debug
FOR /F "tokens=*" %%g IN ('vswhere -latest -requires Microsoft.Component.MSBuild -property productPath') do (SET DEVENVEXE=%%g)
"%DEVENVEXE%" "RESOURCE_NAME_.sln" /build Debug
