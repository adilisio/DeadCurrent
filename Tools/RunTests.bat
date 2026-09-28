@echo off
rem Run the DEAD CURRENT automation tests headless and print a summary.
rem   Tools\RunTests.bat              every test: editor-context tests, then the in-map tests
rem   Tools\RunTests.bat Quest        only DeadCurrent.Quest.* (editor context)
rem   Tools\RunTests.bat -nomap       skip the slower in-map tests
rem   Tools\RunTests.bat -build ...   build DeadCurrentEditor first
rem In-map tests (DeadCurrent.Map.*) run in game context (-game) on Lvl_Boathouse and use a scratch
rem save slot, never the player's. Exit code is 0 only when every test passed.
rem Logs: Saved\Logs\RunTests.log and Saved\Logs\RunTests_Map.log. Set UE_ROOT to override the engine.

setlocal EnableDelayedExpansion
if not defined UE_ROOT set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
set "UE_CMD=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
set "PROJECT=%~dp0..\DeadCurrent.uproject"
set "LOGDIR=%~dp0..\Saved\Logs"
set "COMMON=-unattended -nullrhi -nosound -nosplash -nop4 -log"

set "FILTER="
set "BUILD=0"
set "MAP=1"
:parse
if "%~1"=="" goto parsed
if /I "%~1"=="-build" (set "BUILD=1") else if /I "%~1"=="-nomap" (set "MAP=0") else (set "FILTER=%~1")
shift
goto parse
:parsed

if not exist "%UE_CMD%" (
	echo Unreal Engine not found at "%UE_CMD%". Set UE_ROOT to your UE 5.8 install.
	exit /b 1
)

if "%BUILD%"=="1" (
	call "%UE_ROOT%\Engine\Build\BatchFiles\Build.bat" DeadCurrentEditor Win64 Development -Project="%PROJECT%" -WaitMutex
	if errorlevel 1 (
		echo Build failed.
		exit /b 1
	)
)

set "FAILED=0"
if "%FILTER%"=="" (
	call :run "DeadCurrent" "RunTests.log" ""
	if "%MAP%"=="1" call :run "DeadCurrent.Map" "RunTests_Map.log" "/Game/Maps/Lvl_Boathouse -game"
) else if /I "%FILTER:~0,3%"=="Map" (
	call :run "DeadCurrent.%FILTER%" "RunTests_Map.log" "/Game/Maps/Lvl_Boathouse -game"
) else (
	call :run "DeadCurrent.%FILTER%" "RunTests.log" ""
)

echo.
if "%FAILED%"=="1" (
	echo SOME TESTS FAILED.
	exit /b 1
)
echo All tests passed.
exit /b 0

:run
set "LOG=%LOGDIR%\%~2"
echo Running %~1 tests %~3...
"%UE_CMD%" "%PROJECT%" %~3 %COMMON% "-ExecCmds=Automation RunTests %~1; Quit" -TestExit="Automation Test Queue Empty" -abslog="%LOG%" >nul 2>&1
findstr /C:"Test Completed. Result=" "%LOG%"
findstr /C:"Test Completed. Result={Success" "%LOG%" >nul
if errorlevel 1 (
	echo No tests ran. See %LOG%
	set "FAILED=1"
	exit /b 0
)
findstr /C:"Test Completed. Result={Fail" "%LOG%" >nul
if not errorlevel 1 (
	echo Errors:
	findstr /C:"LogAutomationController: Error" "%LOG%"
	set "FAILED=1"
)
exit /b 0
