@echo off
SETLOCAL

:: Set PATH for 32-bit MinGW
set PATH=C:\mingw32\bin;%PATH%

:: Clean previous builds
echo Cleaning previous 32-bit builds...
scons -c --w32

:: Build 32-bit version
echo Building 32-bit DLL...
scons --w32 -j 12

:: Verify build
if exist "build\obj32\autosar_secacc_32.dll" (
    echo 32-bit build successful!
) else (
    echo 32-bit build failed!
    exit /b 1
)

ENDLOCAL
pause