@echo off
rem The production map registry shared by RunTests, ReviewCapture, Package, and PlayTest. Not run on its own.
rem
rem   call "%~dp0Maps.bat" <map name, alias, or test group>    resolve one map. Sets:
rem        MAP_NAME    Lvl_Boathouse               the level asset (Content\Maps\<MAP_NAME>.umap)
rem        MAP_PATH    /Game/Maps/Lvl_Boathouse    what the engine opens
rem        MAP_GROUP   Boathouse                   the automation group: DeadCurrent.Map.<MAP_GROUP>.*
rem        MAP_EXISTS  1 if the .umap exists, else 0 (a registered map that is not built yet)
rem        MAP_OK      1 if the name was recognised, else 0
rem   call "%~dp0Maps.bat" list                                  every registered map that exists. Sets PRODUCTION_MAPS
rem        (space separated map names, registration order) and PRODUCTION_COUNT, and PRODUCTION_MISSING (registered
rem        production maps with no .umap; RunTests and Package fail on any, so a lost map cannot pass by being skipped).
rem
rem Two kinds of map are registered:
rem   production   Lvl_Boathouse, Lvl_PointeSombre. Listed by "list": cooked by Package.bat, tested by RunTests.bat.
rem   development  Lvl_KitGym (the settlement kit's test ground, Phase 6 WP-KIT). Resolvable by name for ReviewCapture.bat
rem                and PlayTest.bat, but never listed, cooked, or run by the production test loop.
rem Adding a map is one word in the resolve list and, if it is a production map, one in the :list loop. Nothing else
rem in Tools\ changes. The default map (no argument anywhere) is Lvl_Boathouse, so existing habits keep working.
rem No setlocal here on purpose: the caller reads what this sets.

if /I "%~1"=="list" goto :list

set "MAP_OK=0"
set "MAP_NAME="
set "MAP_PATH="
set "MAP_GROUP="
set "MAP_EXISTS=0"
set "WANT=%~1"
if "%WANT%"=="" set "WANT=Lvl_Boathouse"
for %%R in (Lvl_Boathouse:Boathouse Lvl_PointeSombre:Sombre Lvl_KitGym:KitGym) do call :match "%%R"
exit /b 0

:match
for /f "tokens=1,2 delims=:" %%A in ("%~1") do (
	if /I "%WANT%"=="%%A" call :take %%A %%B
	if /I "%WANT%"=="%%B" call :take %%A %%B
)
exit /b 0

:take
set "MAP_OK=1"
set "MAP_NAME=%1"
set "MAP_GROUP=%2"
set "MAP_PATH=/Game/Maps/%1"
set "MAP_EXISTS=0"
if exist "%~dp0..\Content\Maps\%1.umap" set "MAP_EXISTS=1"
exit /b 0

:list
set "PRODUCTION_MAPS="
set "PRODUCTION_MISSING="
set "PRODUCTION_COUNT=0"
for %%R in (Lvl_Boathouse Lvl_PointeSombre) do call :add %%R
exit /b 0

:add
if not exist "%~dp0..\Content\Maps\%1.umap" (
	set "PRODUCTION_MISSING=%PRODUCTION_MISSING% %1"
	exit /b 0
)
set "PRODUCTION_MAPS=%PRODUCTION_MAPS% %1"
set /a PRODUCTION_COUNT+=1
exit /b 0
