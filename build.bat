@echo off
REM ============================================================
REM build.bat - Build script for PrimeToolkit
REM Auto-detects: MinGW g++ > Visual Studio (CMake)
REM ============================================================

cd /d "%~dp0"

set "GPP=C:\Algo Bootstrap\mingw64\bin\g++.exe"

if exist "%GPP%" (
    echo [*] Using MinGW g++
    echo [1/2] Compiling...
    "%GPP%" -std=c++17 -O3 -march=native ^
        -o "PrimeToolkit.exe" ^
        src\core\int128_t.cpp ^
        src\core\primality.cpp ^
        src\core\sieve.cpp ^
        src\core\factorization.cpp ^
        src\server\http_server.cpp ^
        src\main.cpp ^
        -I src ^
        -lws2_32 -lshell32 -static

    if %ERRORLEVEL% NEQ 0 (
        echo [!] Build failed.
        pause
        exit /b 1
    )
    echo [2/2] Build complete!
    echo [+] PrimeToolkit.exe created.
    echo.
    echo Run: PrimeToolkit.exe
    echo.
    pause
    exit /b 0
)

echo [*] Trying CMake + Visual Studio...
where cmake >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [!] Neither MinGW nor CMake found. Install MinGW or Visual Studio.
    pause
    exit /b 1
)

echo [1/4] Configuring CMake...
cmake -B build -G "Visual Studio 17 2022" -A x64
if %ERRORLEVEL% NEQ 0 (
    cmake -B build -G "Visual Studio 18 2022" -A x64
)
if %ERRORLEVEL% NEQ 0 (
    echo [!] CMake configure failed. Ensure Visual Studio 2022 with C++ tools is installed.
    pause
    exit /b 1
)

echo.
echo [2/4] Building Release...
cmake --build build --config Release
if %ERRORLEVEL% NEQ 0 (
    echo [!] Build failed.
    pause
    exit /b 1
)

echo.
echo [3/4] Copying artifacts...
if exist "build\bin\Release\PrimeToolkit.exe" (
    copy /Y "build\bin\Release\PrimeToolkit.exe" "PrimeToolkit.exe" >nul
    echo [+] Copied PrimeToolkit.exe to project root
)
if exist "build\bin\PrimeToolkit.exe" (
    copy /Y "build\bin\PrimeToolkit.exe" "PrimeToolkit.exe" >nul
    echo [+] Copied PrimeToolkit.exe to project root
)

echo.
echo [4/4] Build complete!
echo.
echo Run: PrimeToolkit.exe
echo.
pause