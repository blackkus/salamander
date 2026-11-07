@echo off
setlocal
REM ============================================================
REM  Serveur dedie Salamander (jalon 1)
REM  A VALIDER :
REM   - le serveur lance ici l'executable de l'EDITEUR en mode -server -game
REM     (pas un binaire dedie package : celui-ci exige un moteur compile
REM     depuis les sources, hors perimetre pour l'instant).
REM   - port UDP 7777 : a ouvrir/rediriger sur la box pour jouer depuis
REM     l'exterieur (ou via VPN type Tailscale sur le meme reseau virtuel).
REM ============================================================

REM ===== CHEMINS A ADAPTER =====
set UE_DIR=C:\Games\UE_5.8
set PROJECT_DIR=C:\Users\Blackkus\Documents\Unreal Projects\Salamander
REM =============================

set EDITOR_EXE=%UE_DIR%\Engine\Binaries\Win64\UnrealEditor.exe
set PROJECT=%PROJECT_DIR%\Salamander.uproject
set MAP=/Game/Maps/Monde_Test
set PORT=7777

echo Lancement du serveur dedie : carte %MAP%, port UDP %PORT%
"%EDITOR_EXE%" "%PROJECT%" -server -game -log -PORT=%PORT% "%MAP%?listen"
