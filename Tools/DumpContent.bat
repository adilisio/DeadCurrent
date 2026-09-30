@echo off
rem Dump every generated item, quest, and dialogue asset's properties to Saved\ContentDumps\<label>.json (read-only).
rem   Tools\DumpContent.bat before     then change the generators, run Tools\RebuildContent.bat create_items (etc.),
rem   Tools\DumpContent.bat after      then:  git diff --no-index Saved\ContentDumps\before.json Saved\ContentDumps\after.json
rem An empty diff proves the change kept the generated content's meaningful properties. Close the editor first.
rem Set UE_ROOT to override the engine location. Log: Saved\Logs\DumpContent.log

setlocal
if not defined UE_ROOT set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
set "UE_CMD=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
set "PROJECT=%~dp0..\DeadCurrent.uproject"
set "LABEL=%~1"
if "%LABEL%"=="" set "LABEL=content"

if not exist "%UE_CMD%" (
	echo Unreal Engine not found at "%UE_CMD%". Set UE_ROOT to your UE 5.8 install.
	exit /b 1
)

for %%I in ("%~dp0..\Saved\ContentDumps\%LABEL%.json") do set "DC_DUMP_OUT=%%~fI"
"%UE_CMD%" "%PROJECT%" -run=pythonscript -script="%~dp0EditorScripts\dump_content.py" -unattended -nullrhi -nosplash -log -abslog="%~dp0..\Saved\Logs\DumpContent.log" >nul 2>&1
findstr /C:"Traceback (most recent call last)" "%~dp0..\Saved\Logs\DumpContent.log" >nul
if not errorlevel 1 (
	echo Content dump FAILED. See Saved\Logs\DumpContent.log
	exit /b 1
)
if not exist "%DC_DUMP_OUT%" (
	echo Content dump wrote nothing. See Saved\Logs\DumpContent.log
	exit /b 1
)
echo Wrote %DC_DUMP_OUT%
exit /b 0
