@echo off
setlocal
REM ============================================================
REM  Salamander dedicated server (milestone 1)
REM  TO VALIDATE:
REM   - the server runs the EDITOR executable in -server -game mode
REM     (not a packaged dedicated binary: that requires a source-built engine,
REM     out of scope for now).
REM   - UDP port 7777: forward it on your router to play from outside
REM     (or use a VPN like Tailscale on the same virtual network).
REM ============================================================

REM ===== PATHS TO EDIT =====
set UE_DIR=C:\Games\UE_5.8
set PROJECT_DIR=C:\Users\Blackkus\Documents\Unreal Projects\Salamander
REM =============================

set EDITOR_EXE=%UE_DIR%\Engine\Binaries\Win64\UnrealEditor.exe
set PROJECT=%PROJECT_DIR%\Salamander.uproject
set MAP=/Game/Maps/World_Test
set PORT=7777

echo Starting dedicated server: map %MAP%, UDP port %PORT%
"%EDITOR_EXE%" "%PROJECT%" -server -game -log -PORT=%PORT% "%MAP%?listen"
