@echo off
setlocal
rem Rebuild vendor\watt32-dos from Watt-32 sources using Docker (linux/amd64).
rem Prerequisites: Docker Desktop, Git, network (GitHub + Ubuntu mirrors).
rem Same flow as bootstrap-watt32-docker.sh (uses watt32-docker-build-inner.sh).

cd /d "%~dp0.."
set "ROOT_DIR=%CD%"
set "VENDOR_SRC=%ROOT_DIR%\vendor\watt-32"
set "OUT_DIR=%ROOT_DIR%\vendor\watt32-dos"
set "INNER_SH=%~dp0watt32-docker-build-inner.sh"

where docker >nul 2>&1
if errorlevel 1 (
  echo Docker not found. Install Docker Desktop and ensure docker is on PATH.
  exit /b 1
)
where git >nul 2>&1
if errorlevel 1 (
  echo git not found. Install Git for Windows or add git to PATH.
  exit /b 1
)

if not exist "%INNER_SH%" (
  echo Missing "%INNER_SH%" ^(expected next to this .bat^).
  exit /b 1
)

rem Inner script must be LF-only; Windows CRLF breaks bash inside the Linux container.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0ensure-sh-lf.ps1" -Path "%INNER_SH%"
if errorlevel 1 exit /b 1

echo ==^> Checking Docker daemon ...
docker info >nul 2>&1
if errorlevel 1 (
  echo.
  echo Cannot connect to the Docker engine ^(daemon not running or wrong context^).
  echo   - Start Docker Desktop from the Start menu; wait until it says Docker is running.
  echo   - Errors mentioning dockerDesktopLinuxEngine or npipe usually mean the app is closed
  echo     or still starting; wait 30s after the splash screen, then retry.
  echo   - In Docker Desktop: Settings - General - ensure the WSL2-based engine is enabled if you use it.
  echo   - Run  docker version  from this terminal: you should see both Client and Server blocks.
  echo.
  exit /b 1
)

echo ==^> Cloning Watt-32 into vendor\watt-32 ^(shallow, reproducible^)...
if not exist "%ROOT_DIR%\vendor" mkdir "%ROOT_DIR%\vendor"
if exist "%VENDOR_SRC%" rd /s /q "%VENDOR_SRC%"
git -c core.autocrlf=false -c core.eol=lf clone --depth 1 https://github.com/gvanem/Watt-32.git "%VENDOR_SRC%"
if errorlevel 1 (
  echo git clone failed.
  exit /b 1
)

echo ==^> Building wattcpwl.lib inside Docker ^(Ubuntu + Open Watcom snapshot + DOSBox^)...
docker run --rm --platform linux/amd64 -e SDL_VIDEODRIVER=dummy -v "%VENDOR_SRC%:/watt" -v "%INNER_SH%:/inner.sh:ro" ubuntu:22.04 bash /inner.sh
if errorlevel 1 (
  echo Docker build failed. If you see a pipe / dockerDesktopLinuxEngine error, start Docker Desktop and retry.
  exit /b 1
)

echo ==^> Installing headers + libs into vendor\watt32-dos ...
if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"
if not exist "%OUT_DIR%\lib" mkdir "%OUT_DIR%\lib"
if exist "%OUT_DIR%\inc" rd /s /q "%OUT_DIR%\inc"
xcopy /E /I /Q /Y "%VENDOR_SRC%\inc" "%OUT_DIR%\inc\" >nul
if errorlevel 1 (
  echo xcopy inc failed.
  exit /b 1
)
copy /Y "%VENDOR_SRC%\lib\wattcpwl.lib" "%OUT_DIR%\lib\wattcpwl.lib" >nul
if errorlevel 1 (
  echo copy wattcpwl.lib failed.
  exit /b 1
)
copy /Y "%OUT_DIR%\lib\wattcpwl.lib" "%OUT_DIR%\lib\wattcplf.lib" >nul

echo Done. Run build.bat from the repo root ^(WATT_ROOT defaults to vendor\watt32-dos^).
exit /b 0
