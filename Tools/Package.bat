@echo off
rem Cook and package a Development Win64 build, then smoke-launch every production map.
rem   Tools\Package.bat            build, cook, stage, pak, archive to Saved\Packaged, then a null-RHI smoke launch of each map
rem   Tools\Package.bat -nosmoke   package only
rem Close the editor first. Logs: Saved\Logs\Package.log (packaging), Saved\Logs\PackageSmoke.log (Lvl_Boathouse smoke
rem launch) and Saved\Logs\PackageSmoke_<map>.log (any other production map).
rem The production maps are the registered maps that exist (Tools\Maps.bat): Lvl_Boathouse today, and Lvl_PointeSombre
rem once it is built. They are cooked explicitly (-map=) and each is smoke-loaded: a map that is not in the package,
rem or that logs load errors, fails the run. The smoke launch loads the map with no rendering and exits after a few
rem seconds. It proves the cook is complete and the map loads, not that anything looks right.
rem Set UE_ROOT to override the engine location.

setlocal
set "TOOLS=%~dp0"
if not defined UE_ROOT set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
set "UAT=%UE_ROOT%\Engine\Build\BatchFiles\RunUAT.bat"
for %%I in ("%TOOLS%..") do set "REPO=%%~fI"
set "PROJECT=%REPO%\DeadCurrent.uproject"
set "ARCHIVE=%REPO%\Saved\Packaged"
set "LOG=%REPO%\Saved\Logs\Package.log"

if not exist "%UAT%" (
	echo Unreal Engine not found at "%UAT%". Set UE_ROOT to your UE 5.8 install.
	exit /b 1
)

call "%TOOLS%Maps.bat" list
set "MAPARG="
for %%M in (%PRODUCTION_MAPS%) do call :addmap %%M
echo Production maps:%PRODUCTION_MAPS%

call "%UAT%" BuildCookRun -project="%PROJECT%" -platform=Win64 -clientconfig=Development -build -cook -stage -pak -archive -archivedirectory="%ARCHIVE%" -map=%MAPARG% -nop4 -utf8output -unattended > "%LOG%" 2>&1
if errorlevel 1 (
	echo Packaging failed. See %LOG%
	exit /b 1
)
echo Packaged to %ARCHIVE%\Windows

if /I "%~1"=="-nosmoke" exit /b 0

set "EXE=%ARCHIVE%\Windows\DeadCurrent.exe"
if not exist "%EXE%" (
	echo Packaged executable missing: %EXE%
	exit /b 1
)
for %%M in (%PRODUCTION_MAPS%) do (
	call :smoke %%M
	if errorlevel 1 exit /b 1
)
exit /b 0

rem -map= takes /Game/Maps/<name> joined with +.
:addmap
if defined MAPARG (set "MAPARG=%MAPARG%+/Game/Maps/%1") else (set "MAPARG=/Game/Maps/%1")
exit /b 0

rem Launch the packaged game on one map with no rendering; -ExecCmds quits after the map has ticked for a few seconds.
:smoke
set "SMOKE=%REPO%\Saved\Logs\PackageSmoke_%1.log"
if /I "%1"=="Lvl_Boathouse" set "SMOKE=%REPO%\Saved\Logs\PackageSmoke.log"
"%EXE%" /Game/Maps/%1 -nullrhi -nosound -unattended -nosplash -log -abslog="%SMOKE%" "-ExecCmds=Automation SetMinimumLogVerbosity Log; stat none; Quit" > nul 2>&1
findstr /C:"Error:" "%SMOKE%" | findstr /I /C:"%1" /C:"Failed to load" /C:"Fatal" >nul
if not errorlevel 1 (
	echo Smoke launch of %1 logged load errors. See %SMOKE%
	exit /b 1
)
findstr /C:"LogLoad: Took" "%SMOKE%" | findstr /C:"%1" >nul
if errorlevel 1 (
	echo Smoke launch did not report loading %1. See %SMOKE%
	exit /b 1
)
echo Smoke launch loaded %1.
exit /b 0
