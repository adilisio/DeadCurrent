@echo off
rem Launch DEAD CURRENT for playtesting: standalone game window, no editor, reduced graphics.
rem Uses the compiled editor build, so rebuild DeadCurrentEditor after C++ changes.
rem   Tools\PlayTest.bat                      the project's default map (Lvl_Boathouse)
rem   Tools\PlayTest.bat Lvl_PointeSombre     a specific production map (also accepts Sombre), once that map exists
rem Set UE_ROOT to override the engine location.

if not defined UE_ROOT set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
set "UE_EXE=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor.exe"

if not exist "%UE_EXE%" (
	echo Unreal Engine not found at "%UE_EXE%". Set UE_ROOT to your UE 5.8 install.
	pause
	exit /b 1
)

rem The map is resolved with plain top-level statements, not inside a parenthesised block: cmd expands %VAR% in a block
rem when the block is parsed, before the resolver has run, so a block would never see MAP_OK or MAP_EXISTS.
set "MAPARG="
if "%~1"=="" goto launch
call "%~dp0Maps.bat" %~1
if "%MAP_OK%"=="0" goto unknown
if "%MAP_EXISTS%"=="0" goto notbuilt
set "MAPARG=%MAP_PATH%"
goto launch

:unknown
echo Unknown map "%~1". Registered in Tools\Maps.bat: Lvl_Boathouse, Lvl_PointeSombre, Lvl_KitGym ^(development^).
pause
exit /b 1

:notbuilt
echo %MAP_NAME% is registered but not built yet.
pause
exit /b 1

:launch
rem Low scalability on DX11 (the 1060 handles this much better than DX12).
rem -dpcvars applies before the first frame, so we never start up in Lumen / VSM.
rem Project defaults in DefaultEngine.ini are unchanged.
set "CVARS=r.DynamicGlobalIlluminationMethod=0,r.ReflectionMethod=0,r.Shadow.Virtual.Enable=0,r.VolumetricCloud=0,r.VolumetricFog=0,r.RayTracing=0,r.Lumen.DiffuseIndirect.Allow=0,r.AntiAliasingMethod=0,r.BloomQuality=0,r.MotionBlurQuality=0,r.DepthOfFieldQuality=0,r.LensFlareQuality=0,r.AmbientOcclusionLevels=0,r.DefaultFeature.Bloom=0,r.DefaultFeature.MotionBlur=0,r.ShadowQuality=0,r.Streaming.PoolSize=400,r.ScreenPercentage=70"
start "" "%UE_EXE%" "%~dp0..\DeadCurrent.uproject" %MAPARG% -game -windowed -ResX=1280 -ResY=720 -prefernvidia -dx11 -nosplash -novsync -dpcvars="%CVARS%" ^
	"-ExecCmds=scalability 0, sg.ResolutionQuality 70, t.MaxFPS 60"
