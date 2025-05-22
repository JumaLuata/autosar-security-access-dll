@echo off
SETLOCAL

:: Set PATH for 64-bit MinGW
set PATH=C:\msys64\mingw64\bin;%PATH%

:: Clean previous builds
echo Cleaning previous 64-bit builds...
call scons -c --w64

:: Build 64-bit version
echo Building 64-bit DLL...
call scons --w64 -j 12

:: Verify build
if exist "build\obj64\autosar_secacc_64.dll" (
    echo 64-bit build successful!
) else (
    echo 64-bit build failed!
    exit /b 1
)

ENDLOCAL
pause