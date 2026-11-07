@echo off
setlocal
REM ============================================================
REM  Client Salamander (jalon 1)
REM  Usage : start_client.bat [adresse_ip]
REM    - sans argument, se connecte a 127.0.0.1 (serveur local)
REM    - meme reseau local : IP locale du PC serveur, port UDP 7777
REM    - depuis l'exterieur : IP publique + redirection UDP 7777, ou VPN
REM  Le pseudo et la couleur sont passes en options d'URL :
REM    open <ip>?Name=<pseudo>?Color=<hex>
REM ============================================================

REM ===== CHEMINS A ADAPTER =====
set UE_DIR=C:\Games\UE_5.8
set PROJECT_DIR=C:\Users\Blackkus\Documents\Unreal Projects\Salamander
REM =============================

set SERVER_IP=%1
if "%SERVER_IP%"=="" set SERVER_IP=127.0.0.1

set /p PSEUDO="Pseudo (max 16 caracteres) : "
set /p COULEUR="Couleur hex (ex FF8800) : "

set EDITOR_EXE=%UE_DIR%\Engine\Binaries\Win64\UnrealEditor.exe
set PROJECT=%PROJECT_DIR%\Salamander.uproject

echo Connexion a %SERVER_IP% en tant que %PSEUDO% (couleur %COULEUR%)
"%EDITOR_EXE%" "%PROJECT%" -game -log "%SERVER_IP%?Name=%PSEUDO%?Pseudo=%PSEUDO%?Color=%COULEUR%"
