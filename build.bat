@echo off
setlocal
cd /d "%~dp0"

set "WATCOM=%~dp0watcom"
set "EDPATH=%WATCOM%\eddat"
set "INCLUDE=%WATCOM%\h;%WATCOM%\h\nt"
set "LIB=%WATCOM%\lib286;%WATCOM%\lib286\dos"
set "PATH=%WATCOM%\binnt;%PATH%"

if not exist "%WATCOM%\binnt\wcl.exe" (
  echo Open Watcom compiler not found at "%WATCOM%\binnt\wcl.exe".
  exit /b 1
)

if not exist "%~dp0src\pokyd.c" (
  echo Source file missing: src\pokyd.c
  exit /b 1
)

rem Optional Watt-32 (same layout as build.sh / vendor bundle) for -llm= host mode.
set "LLM_STACK="
set "LLM_CFLAGS="
set "WATT_LIB="
if exist "%~dp0vendor\watt32-dos\inc\tcp.h" (
  if exist "%~dp0vendor\watt32-dos\lib\wattcplf.lib" set "WATT_LIB=%~dp0vendor\watt32-dos\lib\wattcplf.lib"
  if not defined WATT_LIB if exist "%~dp0vendor\watt32-dos\lib\wattcpwl.lib" set "WATT_LIB=%~dp0vendor\watt32-dos\lib\wattcpwl.lib"
  if defined WATT_LIB (
    set "LLM_STACK=-k3584"
    set "LLM_CFLAGS=-DPOKYD_LLM_WATT=1 -I%~dp0vendor\watt32-dos\inc"
    echo Watt-32 found -- LLM mode (-llm=host:port^) will be compiled in.
  ) else (
    echo WARNING: vendor\watt32-dos\inc\tcp.h found but no wattcplf.lib / wattcpwl.lib -- building without LLM.
  )
) else (
  echo WATT_ROOT not set / no vendor\watt32-dos -- building without LLM networking.
)

if exist "%~dp0pokyd.exe" del /q "%~dp0pokyd.exe" >nul 2>&1
if exist "%~dp0src\*.obj" del /q "%~dp0src\*.obj" >nul 2>&1

pushd "%~dp0src"
if defined WATT_LIB goto build_pokyd_watt
wcl -bt=dos -ml -zm -zq -ox -fo=. -fe=..\pokyd.exe pokyd.c
goto build_pokyd_done
:build_pokyd_watt
wcl -bt=dos -ml -zm -zq -ox %LLM_STACK% %LLM_CFLAGS% -fo=. -fe=..\pokyd.exe pokyd.c "%WATT_LIB%"
:build_pokyd_done
set "BUILDERR=%ERRORLEVEL%"
popd

if not "%BUILDERR%"=="0" (
  echo Build failed with exit code %BUILDERR%.
  exit /b %BUILDERR%
)

pushd "%~dp0src"
wcl -bt=dos -ml -zm -zq -ox -fo=. -fe=..\slova.exe slova.c
set "SLVERR=%ERRORLEVEL%"
popd

if not "%SLVERR%"=="0" (
  echo Utility build failed with exit code %SLVERR%.
  exit /b %SLVERR%
)

if not exist "%~dp0pokyd.exe" (
  echo Build finished but pokyd.exe was not created.
  exit /b 1
)

echo Build succeeded: pokyd.exe
if exist "%~dp0slova.exe" echo Utility ready: slova.exe
endlocal
exit /b 0
