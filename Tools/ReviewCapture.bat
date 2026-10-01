@echo off
rem Capture a map from its review viewpoints and write PNGs plus manifest.json.
rem   Tools\ReviewCapture.bat                      Lvl_Boathouse (the default, as always)
rem   Tools\ReviewCapture.bat Lvl_PointeSombre     another production map (also accepts Sombre), once that map exists
rem This is a tool, not part of Tools\RunTests.bat. It fails only when a frame could not be captured.
rem Output: Saved\Review\<yyyy-mm-dd_hhmm>\ for Lvl_Boathouse, Saved\Review\<yyyy-mm-dd_hhmm>_<map>\ for any other map.
rem The views come from Tools\Review\<map>.json plus every Tools\Review\<map>\*.json (one file per content cell).
rem Uses the same window and -dpcvars as Tools\PlayTest.bat. Close the editor first.
rem Same-session A/B pairs (Phase 6 gates) set two environment variables first (cmd splits "=" inside arguments):
rem   set DC_REVIEW_ARGS=-ReviewMaxFPS=0 -ReviewHideTag=Biome   extra engine arguments (uncapped; hide tagged actors)
rem   set DC_REVIEW_LABEL=biome_off                               appended to the output folder name
rem Set UE_ROOT to override the engine location.

setlocal
set "TOOLS=%~dp0"
if not defined UE_ROOT set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
set "UE_EXE=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor.exe"
set "PROJECT=%TOOLS%..\DeadCurrent.uproject"

call "%TOOLS%Maps.bat" %~1
if "%MAP_OK%"=="0" (
	echo Unknown map "%~1". Registered in Tools\Maps.bat: Lvl_Boathouse, Lvl_PointeSombre, Lvl_KitGym ^(development^).
	exit /b 1
)
if "%MAP_EXISTS%"=="0" (
	echo %MAP_NAME% is registered but not built yet.
	exit /b 1
)
if not exist "%TOOLS%Review\%MAP_NAME%.json" if not exist "%TOOLS%Review\%MAP_NAME%\*.json" (
	echo No review views for %MAP_NAME%: add Tools\Review\%MAP_NAME%.json or Tools\Review\%MAP_NAME%\^<cell^>.json
	exit /b 1
)

set "LOG=%TOOLS%..\Saved\Logs\ReviewCapture.log"
if /I not "%MAP_NAME%"=="Lvl_Boathouse" set "LOG=%TOOLS%..\Saved\Logs\ReviewCapture_%MAP_NAME%.log"

if not exist "%UE_EXE%" (
	echo Unreal Engine not found at "%UE_EXE%". Set UE_ROOT to your UE 5.8 install.
	exit /b 1
)

for /f %%i in ('powershell -NoProfile -Command "Get-Date -Format yyyy-MM-dd_HHmm"') do set "STAMP=%%i"
set "SUFFIX="
if /I not "%MAP_NAME%"=="Lvl_Boathouse" set "SUFFIX=_%MAP_NAME%"
if defined DC_REVIEW_LABEL set "SUFFIX=%SUFFIX%_%DC_REVIEW_LABEL%"
for %%I in ("%TOOLS%..\Saved\Review\%STAMP%%SUFFIX%") do set "OUT=%%~fI"
if not exist "%OUT%" mkdir "%OUT%"

rem Same low-spec overrides as PlayTest.bat, applied before the first frame.
set "CVARS=r.DynamicGlobalIlluminationMethod=0,r.ReflectionMethod=0,r.Shadow.Virtual.Enable=0,r.VolumetricCloud=0,r.VolumetricFog=0,r.RayTracing=0,r.Lumen.DiffuseIndirect.Allow=0,r.AntiAliasingMethod=0,r.BloomQuality=0,r.MotionBlurQuality=0,r.DepthOfFieldQuality=0,r.LensFlareQuality=0,r.AmbientOcclusionLevels=0,r.DefaultFeature.Bloom=0,r.DefaultFeature.MotionBlur=0,r.ShadowQuality=0,r.Streaming.PoolSize=400,r.ScreenPercentage=70"

echo Review capture of %MAP_NAME%: %OUT% %DC_REVIEW_ARGS%
"%UE_EXE%" "%PROJECT%" %MAP_PATH% -game -windowed -ResX=1280 -ResY=720 -prefernvidia -dx11 -nosplash -novsync -unattended -dpcvars="%CVARS%" -ReviewDir=%OUT% -ReviewMap=%MAP_PATH% %DC_REVIEW_ARGS% ^
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
rem A material that fails to compile renders as the engine default with no other sign. Presentation Pass
rem playtest 1 found every Meshy prop had been doing that. Any such line in the capture log fails the run.
findstr /C:"Failed to compile Material" "%LOG%" >nul
if not errorlevel 1 (
	echo Review capture found materials that fail to compile ^(they render as the engine default^):
	findstr /C:"Failed to compile Material" "%LOG%" | findstr /C:"Content"
	exit /b 1
)
if not exist "%OUT%\manifest.json" (
	echo Review capture wrote no manifest. See %LOG%
	exit /b 1
)
echo Review capture written to %OUT%
exit /b 0
