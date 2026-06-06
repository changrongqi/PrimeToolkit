@echo off
cd /d "%~dp0"
echo ================================
echo PrimeToolkit - Git Push Script
echo ================================
echo.

:: Check git status
git status
echo.

:: Add all files
echo [1/3] Adding files...
git add .

:: Commit with message
set "msg=Update: %date% %time%"
echo [2/3] Committing with message: %msg%
git commit -m "%msg%"

:: Push to GitHub
echo [3/3] Pushing to GitHub...
git push origin main

if %errorlevel% equ 0 (
    echo.
    echo SUCCESS: Files pushed to GitHub!
) else (
    echo.
    echo ERROR: Push failed. Check your network connection.
)
pause