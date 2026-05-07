@echo off
setlocal
cd /d "%~dp0"

echo Building with Open Watcom ^(Windows host^)...
call "%~dp0build.bat"
if errorlevel 1 (
  echo Build failed.
  exit /b 1
)

echo.
echo Starting DOSBox-X...
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0scripts\run-dosbox.ps1"
set ERR=%ERRORLEVEL%
if not "%ERR%"=="0" (
  echo Run script failed with exit code %ERR%.
  exit /b %ERR%
)

endlocal
exit /b 0
