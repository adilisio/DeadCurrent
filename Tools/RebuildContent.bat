@echo off
rem Regenerate every script-generated asset, in dependency order:
rem items -> quests -> dialogue (references items) -> test gym -> boathouse map -> Pointe Sombre map -> its shoreline
rem recipe (biome\scatter writes Lvl_PointeSombre_Biome and Tools\Biomes\out from the map just built).
rem import_art brings in the CC0 surfaces the boathouse materials resolve; import_audio the CC0 sounds.
rem   Tools\RebuildContent.bat              all of them
rem   Tools\RebuildContent.bat create_quest run one script from Tools\EditorScripts
rem   Tools\RebuildContent.bat kit\build_kit_gym   a script in a subfolder of Tools\EditorScripts (a slash also works)
rem Items, quests, and dialogue are not defined in their create_*.py scripts: they load one spec file per asset group
rem from Tools\ContentSpecs (see its README.md).
rem Close the editor first. Rebuild DeadCurrentEditor after C++ changes before running this.
rem Logs: Saved\Logs\RebuildContent_<script>.log (a subfolder's slash becomes an underscore).
rem Set UE_ROOT to override the engine location.

setlocal
if not defined UE_ROOT set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
set "UE_CMD=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
set "PROJECT=%~dp0..\DeadCurrent.uproject"
set "SCRIPTS=%~dp0EditorScripts"

if not exist "%UE_CMD%" (
	echo Unreal Engine not found at "%UE_CMD%". Set UE_ROOT to your UE 5.8 install.
	exit /b 1
)

rem One script: resolved outside a parenthesised block, so %errorlevel% is read after the run (inside a block it
rem expanded at parse time and a failed script exited 0; found in VS-04).
if not "%~1"=="" goto :one

for %%S in (import_art import_audio create_items create_quest create_dialogue build_test_gym build_boathouse build_pointe_sombre biome\scatter) do (
	call :run %%S
	if errorlevel 1 exit /b 1
)
echo All content regenerated.
exit /b 0

:one
call :run %~1
exit /b %errorlevel%

:run
set "NAME=%~1"
set "NAME=%NAME:/=\%"
set "LOGNAME=%NAME:\=_%"
set "LOG=%~dp0..\Saved\Logs\RebuildContent_%LOGNAME%.log"
echo Running %NAME%...
rem A fresh log every run: if the engine never starts, an old clean log must not read as a pass (Codex, VS-04 audit).
if exist "%LOG%" del /q "%LOG%"
"%UE_CMD%" "%PROJECT%" -run=pythonscript -script="%SCRIPTS%\%NAME%.py" -unattended -nullrhi -nosplash -log -abslog="%LOG%" >nul 2>&1
rem A runtime error prints a Traceback; a syntax error does not (VS-04 found one reported as success), so the
rem commandlet's own "executed with errors" line counts too.
if not exist "%LOG%" (
	echo %NAME% FAILED: the engine wrote no log. %LOG%
	exit /b 1
)
findstr /C:"Traceback (most recent call last)" /C:"Python script executed with errors" "%LOG%" >nul
if not errorlevel 1 (
	echo %NAME% FAILED. See %LOG%
	findstr /C:"LogPython: Error" "%LOG%"
	exit /b 1
)
exit /b 0
