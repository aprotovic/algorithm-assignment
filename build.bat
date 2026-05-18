@echo off
echo Building Broker Management System...
set CXX=g++
where g++ >nul 2>nul
if %errorlevel% neq 0 (
    if exist "%USERPROFILE%\Downloads\winlibs-x86_64-posix-seh-gcc-16.1.0-mingw-w64ucrt-14.0.0-r2\mingw64\bin\g++.exe" (
        set "CXX=%USERPROFILE%\Downloads\winlibs-x86_64-posix-seh-gcc-16.1.0-mingw-w64ucrt-14.0.0-r2\mingw64\bin\g++.exe"
    )
)
"%CXX%" src\main.cpp src\GUI.cpp src\FileStorage.cpp -o BrokerSystem.exe -lgdi32 -lgdiplus -lcomdlg32 -lshell32 -static-libgcc -static-libstdc++
if %errorlevel% neq 0 (
    echo Build failed!
    pause
    exit /b %errorlevel%
)
echo Build successful!
pause
