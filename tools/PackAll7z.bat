@echo off
REM =======================================================
REM ModelExtras Packaging Utility
REM Packs all game files into their proper archives
REM =======================================================

echo ------------------------------------------------------
echo ModelExtras Packaging Utility
echo ------------------------------------------------------

REM Remove old archive folder if exists
if exist "archive" rd /S /Q "archive"
mkdir "archive"

cd tools
echo Packing...

REM Clear temp folder
if exist "pack" rd /S /Q "pack"
mkdir "pack"

REM Copy Release files
call :copyFiles ModelExtras Release

REM Copy Debug files (asi + pdb only)
call :copyFiles ModelExtras Debug


REM Build archive after all files are copied
set "archivePath=..\archive\ModelExtras.7z"
if exist "%archivePath%" del /Q "%archivePath%"
echo Creating 7z archive (8 MB volumes)...
"C:\Program Files\7-Zip\7z.exe" a -t7z "%archivePath%" ".\pack\*" -v8m

REM Cleanup temp folder
if exist "pack" rd /S /Q "pack"

cd ..
echo =======================================================
echo Packaging complete
echo =======================================================
exit /B


:copyFiles
REM Arguments:
REM   %~1 = Module name (ModelExtras)
REM   %~2 = Build config (Release or Debug)

set "moduleName=%~1"
set "buildConfig=%~2"

set "buildPath=..\build\bin\%buildConfig%"
set "srcPath=..\src"
set "folderPath=..\resource\dist\ModelExtras"
set "iniPath=..\resource\dist\ModelExtras.ini"

REM Copy headers, INI, and resource folder ONLY for Release
if /I "%buildConfig%"=="Release" (
    echo Copying headers, INI, and resource folder...
    if exist "%srcPath%\%moduleName%API.h" xcopy /S /Y "%srcPath%\%moduleName%API.h" "pack\" /K /D /H
    if exist "%iniPath%" xcopy /E /Y "%iniPath%" "pack\" /K /D /H
    if exist "%folderPath%" xcopy /E /Y "%folderPath%\*" "pack\%moduleName%\" /K /D /H
)

REM Copy build outputs
echo Copying build files for %buildConfig%...
if exist "%buildPath%\%moduleName%.asi" (
    if /I "%buildConfig%"=="Debug" (
        if not exist "pack\debug_build" mkdir "pack\debug_build"
        xcopy /Y "%buildPath%\%moduleName%.asi" "pack\debug_build\" /K /D /H
    ) else (
        xcopy /Y "%buildPath%\%moduleName%.asi" "pack\" /K /D /H
    )
)
if exist "%buildPath%\%moduleName%.pdb" (
    if /I "%buildConfig%"=="Debug" (
        xcopy /Y "%buildPath%\%moduleName%.pdb" "pack\debug_build\" /K /D /H
    ) else (
        xcopy /Y "%buildPath%\%moduleName%.pdb" "pack\" /K /D /H
    )
)
if /I "%buildConfig%"=="Release" (
    if exist "%buildPath%\%moduleName%.lib" xcopy /Y "%buildPath%\%moduleName%.lib" "pack\" /K /D /H
    if exist "%buildPath%\%moduleName%.exp" xcopy /Y "%buildPath%\%moduleName%.exp" "pack\" /K /D /H
)

echo Finished copying %moduleName% (%buildConfig%)
exit /B
