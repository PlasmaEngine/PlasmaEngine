#define MyAppName "PlasmaLauncher"
#define MyAppNameVisual "Plasma Launcher"
#define MyAppPublisher "Plasma Foundation"
#define MyAppURL "https://plasmagameengine.com"
#define MyAppExeName "PlasmaLauncher.exe"

; Let these defines be defined via command line args if provided
#ifndef PlasmaSource
#define PlasmaSource GetEnv("ZERO_SOURCE")
#endif

#ifndef PlasmaOutput
#define PlasmaOutput GetEnv("ZERO_OUTPUT")
#endif

#ifndef Configuration
#define Configuration "Release"
#endif

#define ExePath "{app}\" + MyAppExeName
#define IconPath "{app}\"
                                
; TestingMode only installs the registry values and sets them to the
; PlasmaOutput directory launcher (the one being built by visual studio)
;#define TestingMode

;this should be replaced via the command line arg /d{define}={value}. Also must be called with iscc.exe, not the .iss file itself.
#ifndef PlasmaLauncherOutputSuffix
#define PlasmaLauncherOutputSuffix "\Out\Win32\" + Configuration + "\PlasmaLauncher"
#endif
#ifndef MajorId
#define MajorId = 1
#endif
#ifndef MyAppVersion
;build the version id up from the major id
#define MyAppVersion MajorId + ".0.0.0"
#endif

; If we're in testing mode then change the exe output path to the built path and look at debug instead of release
#ifdef TestingMode
#define Configuration "Debug"
#define PlasmaLauncherOutputSuffix "\Out\Win32\" + Configuration + "\PlasmaLauncher"
#define ExePath PlasmaOutput + "\" + PlasmaLauncherOutputSuffix + "\" + MyAppExeName
#define IconPath PlasmaSource + "\Projects\Win32Shared\"
#endif

#ifndef PlasmaLauncherOutputPath
#define PlasmaLauncherOutputPath PlasmaOutput + "\" + PlasmaLauncherOutputSuffix
#endif



[Setup]
AppId={{295EE6D2-9E03-43A6-8150-388649CC1341}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
;Don't use app mutex because it doesn't work properly with silent installers
;AppMutex="PlasmaLauncherMutex:{{295EE6D2-9E03-43A6-8150-388649CC1341}"
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
OutputBaseFilename=PlasmaLauncherSetup
SetupIconFile="{#PlasmaSource}\Projects\Win32Shared\PlasmaLauncherIcon.ico"
Compression=lzma
SolidCompression=yes
WizardImageFile=PlasmaInstall.bmp
WizardSmallImageFile=PlasmaLogo.bmp
WizardImageAlphaFormat=defined
ChangesEnvironment=yes
ChangesAssociations=yes
PrivilegesRequired=none
LicenseFile="../License.txt"
CloseApplications=yes
RestartApplications=no
DisableWelcomePage=no
                                                                
[Registry] 
Root: HKCR; Subkey: ".plasmaproj"; ValueType: string; ValueName: ""; ValueData: "PlasmaProject"; Flags: uninsdeletevalue  noerror
Root: HKCR; Subkey: "PlasmaProject"; ValueType: string; ValueName: ""; ValueData: "Plasma Project"; Flags: uninsdeletekey noerror
; Keep the old plasma proj icon instead of the launcher icon (the launcher icon code is commented out below)
Root: HKCR; Subkey: "PlasmaProject\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{#IconPath}PlasmaIcon.ico";   Flags: noerror 
;Root: HKCR; Subkey: "PlasmaProject\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExeName},0";   Flags: noerror 

; Add right-click shell commands to run the project with special commands
Root: HKCR; Subkey: "PlasmaProject\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{#ExePath}"" ""%1"" %*";   Flags: noerror 
Root: HKCR; Subkey: "PlasmaProject\shell\Open With Launcher\command"; ValueType: string; ValueName: ""; ValueData: """{#ExePath}"" ""%1"" -Upgrade";   Flags: noerror 
Root: HKCR; Subkey: "PlasmaProject\shell\Run\command"; ValueType: string; ValueName: ""; ValueData: """{#ExePath}"" ""%1"" -Run";   Flags: noerror 

; Add icons for the right-click shell commands (they pull the icon from the exe)
Root: HKCR; Subkey: "PlasmaProject\shell\open"; ValueType: string; ValueName: "Icon"; ValueData: """{#ExePath}""";   Flags: noerror 
Root: HKCR; Subkey: "PlasmaProject\shell\Open With Launcher"; ValueType: string; ValueName: "Icon"; ValueData: """{#ExePath}""";   Flags: noerror 
Root: HKCR; Subkey: "PlasmaProject\shell\Run"; ValueType: string; ValueName: "Icon"; ValueData: """{#ExePath}""";   Flags: noerror 

; Add registry keys and icons for .plasmabuild files
Root: HKCR; Subkey: ".plasmabuild"; ValueType: string; ValueName: ""; ValueData: "PlasmaBuild"; Flags: uninsdeletevalue  noerror
Root: HKCR; Subkey: "PlasmaBuild"; ValueType: string; ValueName: ""; ValueData: "Plasma Build"; Flags: uninsdeletekey noerror
Root: HKCR; Subkey: "PlasmaBuild\shell\open"; ValueType: string; ValueName: "Icon"; ValueData: """{#ExePath}""";   Flags: noerror 
Root: HKCR; Subkey: "PlasmaBuild\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{#ExePath}"" ""%1"" %*";   Flags: noerror 
Root: HKCR; Subkey: "PlasmaBuild\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{#IconPath}PlasmaIcon.ico";   Flags: noerror 

; Add registry keys and icons for .plasmatemplate files
Root: HKCR; Subkey: ".plasmatemplate"; ValueType: string; ValueName: ""; ValueData: "PlasmaTemplate"; Flags: uninsdeletevalue  noerror
Root: HKCR; Subkey: "PlasmaTemplate"; ValueType: string; ValueName: ""; ValueData: "Plasma Template"; Flags: uninsdeletekey noerror
Root: HKCR; Subkey: "PlasmaTemplate\shell\open"; ValueType: string; ValueName: "Icon"; ValueData: """{#IconPath}PlasmaTemplate.ico""";   Flags: noerror 
Root: HKCR; Subkey: "PlasmaTemplate\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{#ExePath}"" ""%1"" %*";   Flags: noerror 
Root: HKCR; Subkey: "PlasmaTemplate\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{#IconPath}PlasmaTemplate.ico";   Flags: noerror 

; Add registry keys and icons for .plasmaprojpack files
Root: HKCR; Subkey: ".plasmaprojpack"; ValueType: string; ValueName: ""; ValueData: "PlasmaProjPack"; Flags: uninsdeletevalue  noerror
Root: HKCR; Subkey: "PlasmaProjPack"; ValueType: string; ValueName: ""; ValueData: "Plasma Project Package"; Flags: uninsdeletekey noerror
Root: HKCR; Subkey: "PlasmaProjPack\shell\open"; ValueType: string; ValueName: "Icon"; ValueData: """{#IconPath}PlasmaPack.ico""";   Flags: noerror 
Root: HKCR; Subkey: "PlasmaProjPack\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{#ExePath}"" ""%1"" %*";   Flags: noerror 
Root: HKCR; Subkey: "PlasmaProjPack\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{#IconPath}PlasmaPack.ico";   Flags: noerror 

; Add registry keys and icons for .plasmatemplate files
Root: HKCR; Subkey: ".plasmapack"; ValueType: string; ValueName: ""; ValueData: "PlasmaPack"; Flags: uninsdeletevalue  noerror
Root: HKCR; Subkey: "PlasmaPack"; ValueType: string; ValueName: ""; ValueData: "Plasma Pack"; Flags: uninsdeletekey noerror
Root: HKCR; Subkey: "PlasmaPack\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{#IconPath}PlasmaPack.ico";   Flags: noerror 
                               
[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: checkablealone
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 0,6.1

[Icons]
Name: "{group}\{#MyAppNameVisual}"; Filename: "{#ExePath}"
Name: "{group}\Uninstall {#MyAppNameVisual}"; Filename: {uninstallexe}
Name: "{commondesktop}\{#MyAppNameVisual}"; Filename: "{#ExePath}"; Tasks: desktopicon
; Add a startmenu icon to create a new project
Name: "{group}\New Plasma Project"; IconFilename: "{#IconPath}PlasmaLauncherIcon.ico"; Filename: "{#ExePath}"; Parameters: -New; Check: not WizardNoIcons; Tasks: desktopicon

#ifndef TestingMode
[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
;Copy over core resources
Source: "{#PlasmaLauncherOutputPath}\*"; DestDir: "{app}";  Excludes: "*.exp,*.lib,*.ilib,*.ipdb,*.iobj,BuildInfo.data,__pycache__"; Flags: ignoreversion recursesubdirs createallsubdirs
;Copy all icon files over (so we can have different icons for the different shortcuts)
Source: "{#PlasmaSource}\Projects\Win32Shared\*.ico"; DestDir: "{app}"; Flags: ignoreversion

[Run]
Filename: {app}\{#MyAppExeName}; Description: {cm:LaunchProgram,PlasmaLauncher}; Flags: nowait postinstall skipifsilent

;Force the launcher to be closed before installing
[InstallDelete]
Type: filesandordirs; Name: "{app}\{#MyAppExeName}";
Type: filesandordirs; Name: "{localappdata}\PlasmaLauncher_{#MajorId}.0";

[UninstallDelete]
Type: filesandordirs; Name: "{app}\Lib";


[Code] 

function CloseApplicationIfRunning(ApplicationName: String; ApplicationDisplayName: String; WindowName: String) : Boolean;
var
  Confirm: Integer;
  ErrorCode: Integer;
  isSilent: Boolean;
  isPlasmaRunning: Boolean;
begin
  Confirm := IDYES
  isSilent := WizardSilent()
  Result := True

  if isSilent = False then
  begin
    isPlasmaRunning := FindWindowByClassName(WindowName) <> 0
    if isPlasmaRunning = True then
    begin
      Confirm := MsgBox(ApplicationDisplayName + ' is already running. Do you want to close it before uninstalling?', mbConfirmation, MB_YESNO)      
    end;
  end;

  if Confirm = IDYES then                                   
    ShellExec('open', 'taskkill.exe', '/f /im ' + ApplicationName, '', SW_HIDE, ewNoWait, ErrorCode) 	  
  else
    Result := False
end;

function FindAndRemove(Root: Integer; Key: String; ApplicationName: String) : Integer;
var
  uninstaller: String;
  Confirm: Integer;
  ErrorCode: Integer;
  isSilent: Boolean;
begin
  if RegKeyExists(Root, Key) then
  begin
    Confirm := IDYES
    isSilent := WizardSilent()

    if isSilent = False then
    begin
      Confirm := MsgBox(ApplicationName + ' is already installed. Uninstall previous version?', mbConfirmation, MB_YESNO)      
    end;
	  
	  if Confirm = IDYES then
	  begin                                     
	    RegQueryStringValue(Root, Key,'UninstallString', uninstaller);
		  ShellExec('', uninstaller, '/SILENT', '', SW_HIDE, ewWaitUntilTerminated, ErrorCode); 	  
	  end;
  end;
  Result := 0;
end;

function UninstallKey(Key: String; ApplicationName: String): Boolean;
begin
    //Remove 32 bit on 32 bit
    FindAndRemove(HKEY_LOCAL_MACHINE, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{' + Key + '}_is1', ApplicationName);
    //Remove 32 bit on 64 bit
    FindAndRemove(HKEY_LOCAL_MACHINE, 'SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall\{' + Key + '}}_is1', ApplicationName);

    //Remove 32 bit on 32 bit
    FindAndRemove(HKEY_CURRENT_USER, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{' + Key + '}_is1', ApplicationName);
    //Remove 32 bit on 64 bit
    FindAndRemove(HKEY_CURRENT_USER, 'SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall\{' + Key + '}}_is1', ApplicationName);  

    Result := True;
end;
   
function InitializeSetup(): Boolean;
var RegistryKey : String;
begin
    // Legacy. Call the previous uninstaller if the launcher is installed. This shouldn't have to happen unless
    // different registry keys were installed in the past (which they were)
    RegistryKey := '295EE6D2-9E03-43A6-8150-388649CC1341';
    UninstallKey(RegistryKey, 'PlasmaLauncher');

    Result := True;
end;

#endif



