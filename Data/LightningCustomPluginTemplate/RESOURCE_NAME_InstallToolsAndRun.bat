@ECHO OFF
REM Installs the Visual Studio plugin and then runs our solution
PUSHD "%~dp0"
PlasmaLightningPlugins.vsix
DEL /F /Q PlasmaLightninghPlugins.vsix
"RESOURCE_NAME_.bat"
