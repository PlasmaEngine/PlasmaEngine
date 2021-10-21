# [Plasma Engine](https://plasmagameengine.com/)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/80c719056cfe489692ce358756143673)](https://www.codacy.com/gh/PlasmaEngine/PlasmaEngine/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=PlasmaEngine/PlasmaEngine&amp;utm_campaign=Badge_Grade)
[![Windows Build Status](https://travis-ci.org/PlasmaEngine/PlasmaEngine.svg?branch=master)](https://travis-ci.org/PlasmaEngine/PlasmaEngine)

[![Plasma Logo](https://raw.githubusercontent.com/PlasmaEngine/PlasmaEngine/master/GithubMedia/LargeLogo.png)](https://plasmagameengine.com/)

## Overview
Plasma Game engine is a C++ powered 2D and 3D game engine that is designed to be straigt forward yet powerful for all users. We aim to have clear and easy to read code with many comments allowing anyone to pickup and extend the code as they feel is needed. Plasma is origionally branched from the Zero Game Engine.

## Installing binary builds
This is as simple as grabbing the Plasma Launcher from [https://plasmagameengine.com/](https://plasmagameengine.com/)

## Building the engine
Building on Windows Easy (may hit PowerShell permissions issue):
  - Have Visual Studio 2019
  - Download [Bootstrap File](https://github.com/PlasmaEngine/PlasmaEngine/releases/download/NA/Bootstrap.ps1)
  - Place Bootstap in the folder you want Plasma
  - Open PowerShell as Admin and navigate to folder
  - Execute Boostrap
  - Follow Instructions
  
Building on Windows:
  - Install CMake 
  - Install Node.js
  - Do a recusive clone of the git repo
  - Run build_vs2019_x64.bat
  - Open and Build the .sln file located in `Build\Active`

Building on Linux:
  Requires Clang 7.0
  (Note: Dependencies not listed yet)
  - Open terminal in project root
  - run ' node index.js all --alias=Linux '

## Screenshots
![Image of Plasma Engine 3D](https://raw.githubusercontent.com/PlasmaEngine/PlasmaEngine/master/GithubMedia/PlasmaEngine1.PNG)
![Image of Plasma Engine 2D](https://raw.githubusercontent.com/PlasmaEngine/PlasmaEngine/master/GithubMedia/PlasmaEngine2.PNG)

## Credits
  - Josh (Engine Developer)
  - Jesco (Engine Developer)
  - Dave (Engine Contributer)
  - openglfreak (Initial Linux Work)
  - AliFurkan (Linux Work)
  - [AdvancedInstaller](https://www.advancedinstaller.com/) (Providing open source license for launcher installer)
  - Digipen and everyone who worked on Zero
  - WelderFoundation (Useful addition to Zero and thirdparty repos)
