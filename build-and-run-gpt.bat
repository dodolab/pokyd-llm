@echo off
setlocal
cd /d "%~dp0"

rem Windows equivalent of build-and-run-gpt.sh (macOS).
rem Optional: --exit-after-pokyd  --skip-intro

set "FLAGARGS="
:argloop
if "%~1"=="" goto argdone
if /i "%~1"=="--exit-after-pokyd" (
  set "FLAGARGS=%FLAGARGS% -ExitAfterPokyd"
  shift
  goto argloop
)
if /i "%~1"=="--skip-intro" (
  set "FLAGARGS=%FLAGARGS% -SkipIntro"
  shift
  goto argloop
)
echo Unknown argument: %~1
echo Usage: build-and-run-gpt.bat [--exit-after-pokyd] [--skip-intro]
exit /b 1
:argdone

powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0scripts\build-and-run-gpt.ps1"%FLAGARGS%
set "ERR=%ERRORLEVEL%"
if not "%ERR%"=="0" exit /b %ERR%
exit /b 0
