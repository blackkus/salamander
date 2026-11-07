@echo off
setlocal
REM ============================================================
REM  Salamander client (milestone 1)
REM  Usage: start_client.bat [server_ip]
REM    - no argument: connects to 127.0.0.1 (local server)
REM    - same LAN: server PC LAN IP, UDP port 7777
REM    - from outside: public IP + UDP 7777 forward, or VPN
REM  Pseudo and color are passed as URL options:
REM    open <ip>?Name=<pseudo>?Color=<hex>
REM ============================================================

REM ===== PATHS TO EDIT =====
set UE_DIR=C:\Games\UE_5.8
set PROJECT_DIR=C:\Users\Blackkus\Documents\Unreal Projects\Salamander
REM =============================

set SERVER_IP=%1
if "%SERVER_IP%"=="" set SERVER_IP=127.0.0.1

set /p PSEUDO="Pseudo (max 16 chars): "
set /p COULEUR="Color hex (ex FF8800): "

set EDITOR_EXE=%UE_DIR%\Engine\Binaries\Win64\UnrealEditor.exe
set PROJECT=%PROJECT_DIR%\Salamander.uproject

echo Connecting to %SERVER_IP% as %PSEUDO% (color %COULEUR%)
"%EDITOR_EXE%" "%PROJECT%" -game -log "%SERVER_IP%?Name=%PSEUDO%?Pseudo=%PSEUDO%?Color=%COULEUR%"
