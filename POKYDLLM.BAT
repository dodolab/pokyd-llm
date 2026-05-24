@echo off
rem Pokyd with LLM (DOSBox guest). Run from C:\ after NE2000 is loaded.
rem Node bridge must be running on the Windows/Mac host (build-and-run-llm.*).
rem Edit IP/port below if bridge/.env uses a different BRIDGE_PORT.

set POKYD_LLM_IP=10.0.2.2
set POKYD_LLM_PORT=8765


pokyd.exe -consplit -llm %POKYD_LLM_IP% %POKYD_LLM_PORT%
goto KONEC

:KONEC
