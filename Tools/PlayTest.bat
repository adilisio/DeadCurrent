@echo off
rem Launch DEAD CURRENT for playtesting: standalone game window, no editor, reduced graphics.
rem Uses the compiled editor build, so rebuild DeadCurrentEditor after C++ changes.
rem Set UE_ROOT to override the engine location.

if not defined UE_ROOT set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
set "UE_EXE=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor.exe"

if not exist "%UE_EXE%" (
	echo Unreal Engine not found at "%UE_EXE%". Set UE_ROOT to your UE 5.8 install.
	pause
	exit /b 1
)

rem Medium scalability, plus the expensive features Medium leaves on: Lumen GI and reflections
rem (replaced by screen space reflections), virtual shadow maps, and volumetric clouds.
start "" "%UE_EXE%" "%~dp0..\DeadCurrent.uproject" -game -windowed -ResX=1280 -ResY=720 -prefernvidia ^
	"-ExecCmds=scalability 1, sg.ResolutionQuality 100, r.DynamicGlobalIlluminationMethod 0, r.ReflectionMethod 2, r.Shadow.Virtual.Enable 0, r.VolumetricCloud 0, t.MaxFPS 60"
