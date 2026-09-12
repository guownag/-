@echo off
rem ---------------------------------------------------------------------------
rem Wrapper so this project can be built from cmd.exe as well as PowerShell.
rem
rem Why cd first? The project path contains non-ASCII characters. By changing
rem into the project directory and passing only a relative path to PowerShell,
rem we keep the Chinese characters out of the command line entirely.
rem
rem Usage:
rem   build.cmd                    build everything + run all tests
rem   build.cmd -Lesson 01         build + test lesson 01 only
rem   build.cmd -Clean             wipe the build directory first
rem ---------------------------------------------------------------------------
setlocal
cd /d "%~dp0"
powershell -NoProfile -ExecutionPolicy Bypass -File "tools\build.ps1" %*
set RC=%ERRORLEVEL%
endlocal & exit /b %RC%
