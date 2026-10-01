@echo off
rem Run the DEAD CURRENT automation tests headless and print a summary.
rem   Tools\RunTests.bat                   every test: editor-context tests, then the in-map tests of every production map
rem   Tools\RunTests.bat Quest             only DeadCurrent.Quest.* (editor context)
rem   Tools\RunTests.bat Map               only the in-map tests, every production map
rem   Tools\RunTests.bat Map.Boathouse     only DeadCurrent.Map.Boathouse.*, on Lvl_Boathouse
rem   Tools\RunTests.bat Map.Sombre.Vault  only DeadCurrent.Map.Sombre.Vault*, on Lvl_PointeSombre (once that map exists)
rem   Tools\RunTests.bat -nomap            skip the slower in-map tests
rem   Tools\RunTests.bat -build ...        build DeadCurrentEditor first
rem In-map tests (DeadCurrent.Map.<Group>.*) run in game context (-game) on the map registered for that group
rem (Tools\Maps.bat) and use a scratch save slot, never the player's. Exit code is 0 only when every test passed.
rem Logs: Saved\Logs\RunTests.log (editor), RunTests_Map.log (Lvl_Boathouse), RunTests_Map_<Group>.log (other maps).
rem Set UE_ROOT to override the engine.

setlocal EnableDelayedExpansion
if not defined UE_ROOT set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
set "UE_CMD=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
set "PROJECT=%~dp0..\DeadCurrent.uproject"
set "LOGDIR=%~dp0..\Saved\Logs"
set "TOOLS=%~dp0"
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

call "%TOOLS%Maps.bat" list
if defined PRODUCTION_MISSING goto missingmap

set "FAILED=0"
set "F1="
set "F2="
for /f "tokens=1,2 delims=." %%A in ("%FILTER%") do (
	set "F1=%%A"
	set "F2=%%B"
)

if "%FILTER%"=="" (
	call :run "DeadCurrent" "RunTests.log" ""
	if "%MAP%"=="1" call :runall
) else if /I "%F1%"=="Map" (
	if "%F2%"=="" (
		call :runall
	) else (
		call :runfiltered
	)
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

:missingmap
echo Registered production map not built:%PRODUCTION_MISSING%. Run Tools\RebuildContent.bat first.
exit /b 1

rem Every production map that exists, each with its own group.
:runall
for %%M in (%PRODUCTION_MAPS%) do call :runone %%M
exit /b 0

:runone
call "%TOOLS%Maps.bat" %1
call :runmap "DeadCurrent.Map.%MAP_GROUP%"
exit /b 0

rem Map.<Group>[.<rest>]: the filter names its own map through its group.
:runfiltered
call "%TOOLS%Maps.bat" %F2%
if "%MAP_OK%"=="0" (
	echo Unknown map group "%F2%" in "%FILTER%". Registered in Tools\Maps.bat: Boathouse, Sombre, KitGym.
	set "FAILED=1"
	exit /b 0
)
if "%MAP_EXISTS%"=="0" (
	echo %MAP_NAME% is registered but not built yet, so "%FILTER%" cannot run.
	set "FAILED=1"
	exit /b 0
)
call :runmap "DeadCurrent.%FILTER%"
exit /b 0

rem Runs the filter in game context on the map resolved by Maps.bat (MAP_NAME, MAP_PATH, MAP_GROUP).
:runmap
set "MAPLOG=RunTests_Map_%MAP_GROUP%.log"
if /I "%MAP_GROUP%"=="Boathouse" set "MAPLOG=RunTests_Map.log"
call :run %1 "%MAPLOG%" "%MAP_PATH% -game"
exit /b 0

:run
set "LOG=%LOGDIR%\%~2"
echo Running %~1 tests %~3...
rem A fresh log every run: if the engine fails to start, an old log must not read as a new pass.
if exist "%LOG%" del /q "%LOG%"
"%UE_CMD%" "%PROJECT%" %~3 %COMMON% "-ExecCmds=Automation RunTests %~1; Quit" -TestExit="Automation Test Queue Empty" -abslog="%LOG%" >nul 2>&1
if not exist "%LOG%" goto nolog
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

:nolog
echo The engine wrote no log: it did not start. %LOG%
set "FAILED=1"
exit /b 0
