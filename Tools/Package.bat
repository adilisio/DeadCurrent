@echo off
rem Cook and package a Development Win64 build, then smoke-launch it.
rem   Tools\Package.bat            build, cook, stage, pak, archive to Saved\Packaged, then a null-RHI smoke launch
rem   Tools\Package.bat -nosmoke   package only
rem Close the editor first. Logs: Saved\Logs\Package.log (packaging), Saved\Logs\PackageSmoke.log (smoke launch).
rem The smoke launch loads Lvl_Boathouse with no rendering and exits after a few seconds. It proves the cook
rem is complete and the map loads, not that anything looks right. Set UE_ROOT to override the engine location.

setlocal
if not defined UE_ROOT set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
set "UAT=%UE_ROOT%\Engine\Build\BatchFiles\RunUAT.bat"
for %%I in ("%~dp0..") do set "REPO=%%~fI"
set "PROJECT=%REPO%\DeadCurrent.uproject"
set "ARCHIVE=%REPO%\Saved\Packaged"
set "LOG=%REPO%\Saved\Logs\Package.log"

if not exist "%UAT%" (
	echo Unreal Engine not found at "%UAT%". Set UE_ROOT to your UE 5.8 install.
	exit /b 1
)

call "%UAT%" BuildCookRun -project="%PROJECT%" -platform=Win64 -clientconfig=Development -build -cook -stage -pak -archive -archivedirectory="%ARCHIVE%" -nop4 -utf8output -unattended > "%LOG%" 2>&1
if errorlevel 1 (
	echo Packaging failed. See %LOG%
	exit /b 1
)
echo Packaged to %ARCHIVE%\Windows

if /I "%~1"=="-nosmoke" exit /b 0

set "EXE=%ARCHIVE%\Windows\DeadCurrent.exe"
set "SMOKE=%REPO%\Saved\Logs\PackageSmoke.log"
if not exist "%EXE%" (
	echo Packaged executable missing: %EXE%
	exit /b 1
)
rem -ExecCmds quits after the map has ticked for a few seconds.
"%EXE%" /Game/Maps/Lvl_Boathouse -nullrhi -nosound -unattended -nosplash -log -abslog="%SMOKE%" "-ExecCmds=Automation SetMinimumLogVerbosity Log; stat none; Quit" > nul 2>&1
findstr /C:"Error:" "%SMOKE%" | findstr /I /C:"Lvl_Boathouse" /C:"Failed to load" /C:"Fatal" >nul
if not errorlevel 1 (
	echo Smoke launch logged load errors. See %SMOKE%
	exit /b 1
)
findstr /C:"LogLoad: Took" "%SMOKE%" | findstr /C:"Lvl_Boathouse" >nul
if errorlevel 1 (
	echo Smoke launch did not report loading Lvl_Boathouse. See %SMOKE%
	exit /b 1
)
echo Smoke launch loaded Lvl_Boathouse.
exit /b 0
