@echo off
REM Build script for Windows using MSVC

echo Building Concurrent Pizzeria Simulation...
echo.

REM Check if cl.exe is available
where cl.exe >nul 2>&1
if %errorlevel% neq 0 (
    echo Error: MSVC compiler not found. Please run this from a Visual Studio Developer Command Prompt.
    echo.
    echo To fix this:
    echo 1. Open "Developer Command Prompt for VS" or "Developer PowerShell for VS"
    echo 2. Navigate to this directory
    echo 3. Run this script again
    pause
    exit /b 1
)

REM Compile the program
echo Compiling...
cl /EHsc /std:c++20 main.cpp pizzeria.cpp /Fe:pizzeria.exe

REM Check if compilation was successful
if %errorlevel% neq 0 (
    echo.
    echo Compilation failed!
    pause
    exit /b 1
)

echo.
echo Compilation successful!
echo.
echo To run the simulation, execute: pizzeria.exe
echo.
pause
