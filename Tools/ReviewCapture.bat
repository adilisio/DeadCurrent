@echo off
rem Capture Lvl_Boathouse from the review viewpoints and write PNGs plus manifest.json.
rem This is a tool, not part of Tools\RunTests.bat. It fails only when a frame could not be captured.
rem Output: Saved\Review\<yyyy-mm-dd_hhmm>\
rem Uses the same window and -dpcvars as Tools\PlayTest.bat. Close the editor first.
rem Set UE_ROOT to override the engine location.

setlocal
if not defined UE_ROOT set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
set "UE_EXE=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor.exe"
set "PROJECT=%~dp0..\DeadCurrent.uproject"
set "LOG=%~dp0..\Saved\Logs\ReviewCapture.log"

if not exist "%UE_EXE%" (
	echo Unreal Engine not found at "%UE_EXE%". Set UE_ROOT to your UE 5.8 install.
	exit /b 1
)

for /f %%i in ('powershell -NoProfile -Command "Get-Date -Format yyyy-MM-dd_HHmm"') do set "STAMP=%%i"
for %%I in ("%~dp0..\Saved\Review\%STAMP%") do set "OUT=%%~fI"
if not exist "%OUT%" mkdir "%OUT%"

rem Same low-spec overrides as PlayTest.bat, applied before the first frame.
set "CVARS=r.DynamicGlobalIlluminationMethod=0,r.ReflectionMethod=0,r.Shadow.Virtual.Enable=0,r.VolumetricCloud=0,r.VolumetricFog=0,r.RayTracing=0,r.Lumen.DiffuseIndirect.Allow=0,r.AntiAliasingMethod=0,r.BloomQuality=0,r.MotionBlurQuality=0,r.DepthOfFieldQuality=0,r.LensFlareQuality=0,r.AmbientOcclusionLevels=0,r.DefaultFeature.Bloom=0,r.DefaultFeature.MotionBlur=0,r.ShadowQuality=0,r.Streaming.PoolSize=400,r.ScreenPercentage=70"

echo Review capture: %OUT%
"%UE_EXE%" "%PROJECT%" /Game/Maps/Lvl_Boathouse -game -windowed -ResX=1280 -ResY=720 -prefernvidia -dx11 -nosplash -novsync -unattended -dpcvars="%CVARS%" -ReviewDir=%OUT% ^
	"-ExecCmds=Automation RunTests DeadCurrent.Review.Capture; Quit" -TestExit="Automation Test Queue Empty" -abslog="%LOG%"

findstr /C:"Test Completed. Result={Success}" "%LOG%" >nul
if errorlevel 1 (
	echo Review capture failed. See %LOG%
	exit /b 1
)
findstr /C:"Test Completed. Result={Fail" "%LOG%" >nul
if not errorlevel 1 (
	echo Review capture failed. See %LOG%
	exit /b 1
)
if not exist "%OUT%\manifest.json" (
	echo Review capture wrote no manifest. See %LOG%
	exit /b 1
)
echo Review capture written to %OUT%
exit /b 0
