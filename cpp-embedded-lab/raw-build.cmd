@echo off
rem ---------------------------------------------------------------------------
rem Lesson 00 helper: walk through preprocess -> compile -> assemble -> link
rem by hand, without CMake. See lessons\00-toolchain\README.md.
rem ---------------------------------------------------------------------------
setlocal
cd /d "%~dp0"
powershell -NoProfile -ExecutionPolicy Bypass -File "tools\raw-build.ps1" %*
set RC=%ERRORLEVEL%
endlocal & exit /b %RC%
