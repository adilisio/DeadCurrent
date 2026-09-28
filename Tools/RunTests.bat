@echo off
rem Run the DEAD CURRENT automation tests headless and print a summary.
rem   Tools\RunTests.bat            run every DeadCurrent.* test
rem   Tools\RunTests.bat Quest      run DeadCurrent.Quest.*
rem   Tools\RunTests.bat -build     build DeadCurrentEditor first, then run every test
rem Exit code is 0 only when every test passed. Full log: Saved\Logs\RunTests.log
rem Set UE_ROOT to override the engine location.

setlocal
if not defined UE_ROOT set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
set "UE_CMD=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
set "PROJECT=%~dp0..\DeadCurrent.uproject"
set "LOG=%~dp0..\Saved\Logs\RunTests.log"

set "FILTER=DeadCurrent"
if /I "%~1"=="-build" (
	call "%UE_ROOT%\Engine\Build\BatchFiles\Build.bat" DeadCurrentEditor Win64 Development -Project="%PROJECT%" -WaitMutex
	if errorlevel 1 (
		echo Build failed.
		exit /b 1
	)
) else if not "%~1"=="" (
	set "FILTER=DeadCurrent.%~1"
)

if not exist "%UE_CMD%" (
	echo Unreal Engine not found at "%UE_CMD%". Set UE_ROOT to your UE 5.8 install.
	exit /b 1
)

echo Running %FILTER% tests...
"%UE_CMD%" "%PROJECT%" -unattended -nullrhi -nosound -nosplash -nop4 "-ExecCmds=Automation RunTests %FILTER%; Quit" -TestExit="Automation Test Queue Empty" -log -abslog="%LOG%" >nul 2>&1

findstr /C:"Test Completed. Result=" "%LOG%"
findstr /C:"Test Completed. Result={Fail" "%LOG%" >nul
if not errorlevel 1 (
	echo.
	echo FAILED. Errors:
	findstr /R /C:"LogAutomationController: Error" "%LOG%"
	exit /b 1
)
findstr /C:"Test Completed. Result={Success" "%LOG%" >nul
if errorlevel 1 (
	echo No tests ran. See %LOG%
	exit /b 1
)
echo.
echo All tests passed.
exit /b 0
