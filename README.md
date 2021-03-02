# [Plasma Engine](https://plasmagameengine.com/)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/80c719056cfe489692ce358756143673)](https://www.codacy.com/gh/PlasmaEngine/PlasmaEngine/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=PlasmaEngine/PlasmaEngine&amp;utm_campaign=Badge_Grade)
[![Windows Build Status (Does not currently represent windows build process)](https://travis-ci.org/PlasmaEngine/PlasmaEngine.svg?branch=master)](https://travis-ci.org/PlasmaEngine/PlasmaEngine)

[![Plasma Logo](https://raw.githubusercontent.com/PlasmaEngine/PlasmaEngine/master/GithubMedia/LargeLogo.png)](https://plasmagameengine.com/)

## Overview
Plasma Game engine is a C++ powered 2D and 3D game engine that is designed to be straigt forward yet powerful for all users. Plasma also aims to maintain as much freedom as possible for the engine and tools, this is done by building as many features as possible ourselves without depending on 3rd party libraries.

## Installing binary builds
This is as simple as grabbing the Plasma Launcher from [https://plasmagameengine.com/](https://plasmagameengine.com/)

## Building the engine
Building on Windows Easy:
  - Have Visual Studio 2019
  - Download [Bootstrap File](https://github.com/PlasmaEngine/PlasmaEngine/releases/download/NA/Bootstrap.ps1)
  - Place Bootstap in the folder you want Plasma
  - Open PowerShell as Admin and navigate to folder
  - Execute Boostrap
  - Follow Instructions
  
Building on Windows:
  - Install CMake and Node.js
  - Do a recusive clone of the git repo
  - Run init.bat
  - Run GenerateVS2019.bat
  - Open and Build the .sln file located in `Build\Active`

## Key Features

  - Custom Scripting Language called Lightning (a friend called it js mixed with C#)
  - Custom Physics Engine
  - Nested Archetypes (essentially nested prefabs)
  - Custom Shader language
  - One Click Network Replication
  - Scriptable Render Pipeline
  - Tilemap Editor

## Screenshots
![Image of Plasma Engine 3D](https://raw.githubusercontent.com/PlasmaEngine/PlasmaEngine/master/GithubMedia/PlasmaEngine1.PNG)
![Image of Plasma Engine 2D](https://raw.githubusercontent.com/PlasmaEngine/PlasmaEngine/master/GithubMedia/PlasmaEngine2.PNG)

## Credits
  - Josh (Engine Developer)
  - Jesco (Engine Developer)
  - Dave (Engine Contributer)
  - [AdvancedInstaller](https://www.advancedinstaller.com/) (Providing open source licecne for launcher installer)
  - Digipen (Built Zero Engine)
  - WelderFoundation (Useful addition to Zero and thirdparty repos)
