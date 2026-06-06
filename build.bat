@echo off
REM ============================================================
REM build.bat - Build script for PrimeToolkit
REM Supports: MinGW g++, MSVC (via CMake), Clang
REM ============================================================

cd /d "%~dp0"

REM --- Step 1: Try g++ from PATH ---
where g++ >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo [*] Using g++ from PATH
    echo [1/2] Compiling...
    g++ -std=c++17 -O3 -march=x86-64 ^
        -o "build\bin\PrimeToolkit.exe" ^
        src\core\int128_t.cpp ^
        src\core\primality.cpp ^
        src\core\sieve.cpp ^
        src\core\factorization.cpp ^
        src\api.cpp ^
        src\server\http_server.cpp ^
        src\main.cpp ^
        -I src ^
        -lws2_32

    if %ERRORLEVEL% EQU 0 (
        echo [2/2] Build complete!
        echo [+] build\bin\PrimeToolkit.exe created.
        echo.
        echo Run: build\bin\PrimeToolkit.exe
        echo.
        goto :done
    )
)

REM --- Step 2: Try common MinGW paths ---
set "MIN_PATHS=C:\mingw64\bin;C:\msys64\mingw64\bin;C:\msys64\mingw64\bin;C:\Program Files\mingw-w64\x86_64-8.1.0-posix-seh-rt_v6-rev0\mingw64\bin"

for %%P in (%MIN_PATHS%) do (
    if exist "%%P\g++.exe" (
        echo [*] Using MinGW from %%P
        echo [1/2] Compiling...
        "%%P\g++.exe" -std=c++17 -O3 -march=x86-64 ^
            -o "build\bin\PrimeToolkit.exe" ^
            src\core\int128_t.cpp ^
            src\core\primality.cpp ^
            src\core\sieve.cpp ^
            src\core\factorization.cpp ^
            src\api.cpp ^
            src\server\http_server.cpp ^
            src\main.cpp ^
            -I src ^
            -lws2_32

        if %ERRORLEVEL% EQU 0 (
            echo [2/2] Build complete!
            echo [+] build\bin\PrimeToolkit.exe created.
            echo.
            echo Run: build\bin\PrimeToolkit.exe
            echo.
            goto :done
        )
    )
)

REM --- Step 3: Fall back to CMake ---
echo [*] Trying CMake...
where cmake >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [!] No compiler found. Install MinGW or Visual Studio with C++ tools.
    echo.
    echo Install MinGW: https://www.mingw-w64.org/
    echo Or use Visual Studio Installer to add "Desktop development with C++"
    pause
    exit /b 1
)

echo [1/4] Cleaning old build...
if exist "build" rmdir /s /q build

echo [2/4] Configuring CMake (MinGW Makefiles)...
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
if %ERRORLEVEL% NEQ 0 (
    echo [2/4] Trying CMake with default generator...
    cmake -B build -DCMAKE_BUILD_TYPE=Release
)
if %ERRORLEVEL% NEQ 0 (
    echo [!] CMake configure failed.
    pause
    exit /b 1
)

echo [3/4] Building...
cmake --build build --config Release
if %ERRORLEVEL% NEQ 0 (
    echo [!] Build failed.
    pause
    exit /b 1
)

echo [4/4] Copying web/ to output...
if not exist "build\bin" mkdir build\bin
xcopy /e /q "web" "build\bin\web\" >nul 2>&1

echo.
echo [+] Build complete!
echo [+] build\bin\PrimeToolkit.exe created.
echo.
echo Run: build\bin\PrimeToolkit.exe
echo.

:done
pause
