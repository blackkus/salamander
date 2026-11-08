@echo off
setlocal
REM ============================================================
REM  Salamander client — opens the in-game MAIN MENU (Slate):
REM  enter server IP, nick, color and key bindings there, then Connect.
REM  Server IP: no arg -> 127.0.0.1 ; same LAN -> server PC LAN IP ;
REM  from outside -> public IP with UDP 7777 forwarded, or a VPN (Tailscale).
REM  (CLI tests can still bypass the menu: launch with "<ip>?Nick=X?Color=FF8800"
REM   or -ConnectTo=<ip> -ConnectNick=X -ConnectColor=FF8800)
REM ============================================================

REM ===== PATHS TO EDIT =====
set UE_DIR=C:\Games\UE_5.8
set PROJECT_DIR=C:\Users\Blackkus\Documents\Unreal Projects\Salamander
REM =============================

set EDITOR_EXE=%UE_DIR%\Engine\Binaries\Win64\UnrealEditor.exe
set PROJECT=%PROJECT_DIR%\Salamander.uproject

echo Opening Salamander main menu (fill server IP / nick / color, then Connect)
"%EDITOR_EXE%" "%PROJECT%" -game -log
