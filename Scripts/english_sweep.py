# -*- coding: utf-8 -*-
# One-shot sweep: replace French comments/log strings in code files by English ones.
import os, re, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

PAIRS = [
    # logs / user-facing strings
    ('"Pseudo required: open <ip>?Name=Bob?Color=FF8800"', '"Pseudo required: open <ip>?Name=Bob?Color=FF8800"'),
    ('"Pseudo too long (16 chars max)"', '"Pseudo too long (16 chars max)"'),
    ('"Pseudo already taken"', '"Pseudo already taken"'),
    ('"[GameMode] Loaded profile for %s: Level=%d"', '"[GameMode] Loaded profile for %s: Level=%d"'),
    ('"[GameMode] Saved profile: %s (Level=%d)"', '"[GameMode] Saved profile: %s (Level=%d)"'),
    ('"[GameMode] %s died (HP=0)"', '"[GameMode] %s died (HP=0)"'),
    ('"[GameMode] %s respawned at start point"', '"[GameMode] %s respawned at start point"'),
    ('"[GameMode] Connecting to %s (PC=%d)"', '"[GameMode] Connecting to %s (PC=%d)"'),
    ('"[GameMode] Connecting to %s"', '"[GameMode] Connecting to %s"'),
    ('"[GameMode] Login menu shown"', '"[GameMode] Login menu shown"'),
    ('"[Char] PlayerState received: Name=%s Level=%d"', '"[Char] PlayerState received: Name=%s Level=%d"'),
    ('"[Move] server correction err=%.1f cm"', '"[Move] server correction err=%.1f cm"'),
    ('"[PS] M_PlayerColor not found"', '"[PS] M_PlayerColor not found"'),
    ('"Server IP (ex 127.0.0.1 or server PC LAN IP)"', '"Server IP (ex 127.0.0.1 or server PC LAN IP)"'),
    ('"Pseudo (16 chars max)"', '"Pseudo (16 chars max)"'),
    ('"Color hex (ex FF8800)"', '"Color hex (ex FF8800)"'),
    ('"Connect"', '"Connect"'),
]

COMMENT_PAIRS = [
    ("In CLI (-game), the engine overwrites the Name option with the local nickname; so we read", "In CLI (-game), the engine overwrites the Name option with the local nickname; so we read"),
    ("l'option \"Pseudo\" en priorite (passe par start_client.bat), \"Name\" en fallback (éditeur).", "the \"Pseudo\" option first (sent by start_client.bat), \"Name\" as fallback (editor)."),
    ("Rejects the connection if the pseudo (?Name= option) is empty, > 16 chars or already taken.", "Rejects the connection if the pseudo (?Name= option) is empty, > 16 chars or already taken."),
    ("Reads ?Pseudo=/?Name= and ?Color= from the connection URL and stores them in the PlayerState (server).", "Reads ?Pseudo=/?Name= and ?Color= from the connection URL and stores them in the PlayerState (server)."),
    ("Login screen (Menu map, local client only).", "Login screen (Menu map, local client only)."),
    ("Server console command: applies damage to all connected players.", "Server console command: applies damage to all connected players."),
    ("Applies damage to all pawns (server only).", "Applies damage to all pawns (server only)."),
    ("Server-side save (one file per player).", "Server-side save (one file per player)."),
    ("Player death (HP = 0): destroys the pawn and respawns at the start point after 3 s.", "Player death (HP = 0): destroys the pawn and respawns at the start point after 3 s."),
    ("already rejected by the parent", "already rejected by the parent"),
    ("Load the saved profile (server only): level and color are restored.", "Load the saved profile (server only): level and color are restored."),
    ("server side (clients get it via OnRep_Color)", "server side (clients get it via OnRep_Color)"),
    ("Periodic save every 60 s.", "Periodic save every 60 s."),
    ("Save backend: files on the server disk, one per player.", "Save backend: files on the server disk, one per player."),
    ("Menu map (client started without IP): no pawn, show the Slate login screen.", "Menu map (client started without IP): no pawn, show the Slate login screen."),
    ("CLI test: -ConnectTo=127.0.0.1 -ConnectPseudo=Bob -ConnectColor=FF8800 -> auto-connect without click.", "CLI test: -ConnectTo=127.0.0.1 -ConnectPseudo=Bob -ConnectColor=FF8800 -> auto-connect without click."),
    ("Automated CLI tests (server started with -TestAutoDamage=<n> and/or -TestAutoLevel=<n>).", "Automated CLI tests (server started with -TestAutoDamage=<n> and/or -TestAutoLevel=<n>)."),
    ("Save on disconnect.", "Save on disconnect."),
    ("Respawn at the start point; the new pawn comes back with full HP (no penalty).", "Respawn at the start point; the new pawn comes back with full HP (no penalty)."),
    ("Server console only (dedicated server console).", "Server console only (dedicated server console)."),
    ("Standard menu approach: client travel to the URL (IP?options).", "Standard menu approach: client travel to the URL (IP?options)."),
    ("The PlayerController stays the template Third Person one (Blueprint untouched).", "The PlayerController stays the template Third Person one (Blueprint untouched)."),
    ("GAS: bind the PlayerState ASC to this pawn (owner=PlayerState, avatar=pawn).", "GAS: bind the PlayerState ASC to this pawn (owner=PlayerState, avatar=pawn)."),
    ("Init attributes (Health/MaxHealth = 100) via GameplayEffect, server only.", "Init attributes (Health/MaxHealth = 100) via GameplayEffect, server only."),
    ("client side, on PlayerState replication", "client side, on PlayerState replication"),
    ("CLI tests (client only): auto play sprint / crouch after connecting.", "CLI tests (client only): auto play sprint / crouch after connecting."),
    ("CLI test: auto move (simulates holding W).", "CLI test: auto move (simulates holding W)."),
    ("Mesh and AnimBP from the Third Person template (existing assets, untouched).", "Mesh and AnimBP from the Third Person template (existing assets, untouched)."),
    ("Input assets from the template + IA_Sprint / IA_Crouch (created by editor script).", "Input assets from the template + IA_Sprint / IA_Crouch (created by editor script)."),
    ("IA_Sprint / IA_Crouch are created by an editor script; the key mapping", "IA_Sprint / IA_Crouch are created by an editor script; the key mapping"),
    ("is added here in code (idempotent thanks to UnmapKey).", "is added here in code (idempotent thanks to UnmapKey)."),
    ("Native crouch of CharacterMovementComponent (bWantsToCrouch, predicted).", "Native crouch of CharacterMovementComponent (bWantsToCrouch, predicted)."),
    ("Move custom : transporte le flag \"sprint\" dans les bits compressés,", "Custom move: carries the \"sprint\" flag in the compressed bits,"),
    ("like bWantsToCrouch, so the sprint speed is predicted on the client.", "like bWantsToCrouch, so the sprint speed is predicted on the client."),
    ("We fill the parent class ClientPredictionData member (like the engine does),", "We fill the parent class ClientPredictionData member (like the engine does),"),
    ("because the engine accesses it directly after the call.", "because the engine accesses it directly after the call."),
    ("Max speed while sprinting (base walk/run = MaxWalkSpeed, unchanged).", "Max speed while sprinting (base walk/run = MaxWalkSpeed, unchanged)."),
    ("Predicted: packed into FSavedMove_Salamander and replayed on client and server.", "Predicted: packed into FSavedMove_Salamander and replayed on client and server."),
    ("Logs the magnitude of each server -> client correction (prediction test).", "Logs the magnitude of each server -> client correction (prediction test)."),
    ("Current HP. Any change goes through a GameplayEffect (server).", "Current HP. Any change goes through a GameplayEffect (server)."),
    ("Server side: called after each GameplayEffect -> detects death (Health <= 0).", "Server side: called after each GameplayEffect -> detects death (Health <= 0)."),
    ("Death: notify the GameMode, server side only.", "Death: notify the GameMode, server side only."),
    ("Sets Health and MaxHealth to 100 (Instant, Override). Applied server-side on possession.", "Sets Health and MaxHealth to 100 (Instant, Override). Applied server-side on possession."),
    ("Instant damage: Health += SetByCaller(Data.Damage). Applied on the server only.", "Instant damage: Health += SetByCaller(Data.Damage). Applied on the server only."),
    ("GAS: the ASC lives on the PlayerState (survives death/respawn). Mixed replication.", "GAS: the ASC lives on the PlayerState (survives death/respawn). Mixed replication."),
    ("Pseudo chosen at connect (URL option ?Pseudo= / ?Name=...). Server only.", "Pseudo chosen at connect (URL option ?Pseudo= / ?Name=...). Server only."),
    ("Color chosen at connect (?Color=RRGGBB), applied to the mesh.", "Color chosen at connect (?Color=RRGGBB), applied to the mesh."),
    ("Creates a MID from M_PlayerColor (BodyColor param) and applies it to the pawn mesh.", "Creates a MID from M_PlayerColor (BodyColor param) and applies it to the pawn mesh."),
    ("Player level. Server only.", "Player level. Server only."),
    ("Saved player profile, server side only.", "Saved player profile, server side only."),
    ("Save format version (migration via ISaveBackend::MigrateProfile).", "Save format version (migration via ISaveBackend::MigrateProfile)."),
    ("Save backend interface (one file per player on the server disk).", "Save backend interface (one file per player on the server disk)."),
    ("Migration of an old-version save (empty for now).", "Migration of an old-version save (empty for now)."),
    ("Save into USaveGame files, one per player (slot = key), on the server disk.", "Save into USaveGame files, one per player (slot = key), on the server disk."),
    ("The key is the pseudo: deliberately simple and NOT secure.", "The key is the pseudo: deliberately simple and NOT secure."),
    ("TODO(identity): replace with a unique account id before any public release.", "TODO(identity): replace with a unique account id before any public release."),
    ("old-version migration (empty for now)", "old-version migration (empty for now)"),
    ("USaveGame is not copyable via operator=: copy field by field.", "USaveGame is not copyable via operator=: copy field by field."),
    ("Login screen (Menu map): IP + pseudo + color + button. 100% Slate, no Blueprint.", "Login screen (Menu map): IP + pseudo + color + button. 100% Slate, no Blueprint."),
    ("Creates a named vector parameter node in a material (called from editor Python scripts,", "Creates a named vector parameter node in a material (called from editor Python scripts,"),
    ("because ParameterName/DefaultValue are not exposed to Python in 5.8).", "because ParameterName/DefaultValue are not exposed to Python in 5.8)."),
    # .bat REM comments
    ("Salamander dedicated server (milestone 1)", "Salamander dedicated server (milestone 1)"),
    ("TO VALIDATE:", "TO VALIDATE:"),
    ("the server runs the EDITOR executable in -server -game mode", "the server runs the EDITOR executable in -server -game mode"),
    ("(not a packaged dedicated binary: that requires a source-built engine,", "(not a packaged dedicated binary: that requires a source-built engine,"),
    ("out of scope for now).", "out of scope for now)."),
    ("UDP port 7777: forward it on your router to play from outside", "UDP port 7777: forward it on your router to play from outside"),
    ("(or use a VPN like Tailscale on the same virtual network).", "(or use a VPN like Tailscale on the same virtual network)."),
    ("PATHS TO EDIT", "PATHS TO EDIT"),
    ("Starting dedicated server: map %MAP%, UDP port %PORT%", "Starting dedicated server: map %MAP%, UDP port %PORT%"),
    ("Salamander client (milestone 1)", "Salamander client (milestone 1)"),
    ("Usage: start_client.bat [server_ip]", "Usage: start_client.bat [server_ip]"),
    ("- no argument: connects to 127.0.0.1 (local server)", "- no argument: connects to 127.0.0.1 (local server)"),
    ("- same LAN: server PC LAN IP, UDP port 7777", "- same LAN: server PC LAN IP, UDP port 7777"),
    ("- from outside: public IP + UDP 7777 forward, or VPN", "- from outside: public IP + UDP 7777 forward, or VPN"),
    ("Pseudo and color are passed as URL options:", "Pseudo and color are passed as URL options:"),
    ('"Pseudo (max 16 chars): "', '"Pseudo (max 16 chars): "'),
    ('"Color hex (ex FF8800): "', '"Color hex (ex FF8800): "'),
    ("Connecting to %SERVER_IP% as %PSEUDO% (color %COULEUR%)", "Connecting to %SERVER_IP% as %PSEUDO% (color %COULEUR%)"),
    # python scripts comments
    ("Creates the IA_Sprint / IA_Crouch Input Actions", "Creates the IA_Sprint / IA_Crouch Input Actions"),
    ("The key mapping (LeftShift / LeftControl) is added in C++ in ASalamanderCharacter.", "The key mapping (LeftShift / LeftControl) is added in C++ in ASalamanderCharacter."),
    ("Creates M_PlayerColor (BodyColor vector param), M_SolGreen, and the /Game/Maps/Monde_Test map", "Creates M_PlayerColor (BodyColor vector param), M_SolGreen, and the /Game/Maps/Monde_Test map"),
    ("(green floor, SkyAtmosphere, sun, SkyLight, PlayerStart). Idempotent.", "(green floor, SkyAtmosphere, sun, SkyLight, PlayerStart). Idempotent."),
    ("ParameterName is not exposed in Python 5.8: C++ helper.", "ParameterName is not exposed in Python 5.8: C++ helper."),
    ("Creates the /Game/Maps/Menu map (empty: the Slate login screen is shown on top).", "Creates the /Game/Maps/Menu map (empty: the Slate login screen is shown on top)."),
    ("Menu map already exists", "Menu map already exists"),
    ("Menu map created: ", "Menu map created: "),
    ("IA_Sprint and IA_Crouch OK in /Game/Input/Actions", "IA_Sprint and IA_Crouch OK in /Game/Input/Actions"),
    ("Monde_Test OK: green floor, blue sky, sun, SkyLight, PlayerStart + M_PlayerColor/M_SolGreen", "Monde_Test OK: green floor, blue sky, sun, SkyLight, PlayerStart + M_PlayerColor/M_SolGreen"),
    ("value_type est read-only :\n# il est fixe par le factory a la creation (BOOLEAN par defaut pour InputAction_Factory).", "value_type is read-only:\n# it is set by the factory at creation (BOOLEAN by default for InputAction_Factory)."),
]

ALL = PAIRS + COMMENT_PAIRS

files = []
for root in ('Source', 'Scripts'):
    for dirpath, _, names in os.walk(os.path.join(ROOT, root)):
        for n in names:
            if n.endswith(('.h', '.cpp', '.cs', '.py', '.bat')):
                files.append(os.path.join(dirpath, n))

changed = 0
for f in files:
    s = open(f, encoding='utf-8').read()
    orig = s
    for old, new in ALL:
        s = s.replace(old, new)
    if s != orig:
        open(f, 'w', encoding='utf-8').write(s)
        changed += 1
print('FILES_CHANGED', changed, 'of', len(files))
for f in files:
    for i, line in enumerate(open(f, encoding='utf-8'), 1):
        if re.search(r'[àâäéèêëîïôöùûüç]', line):
            print('FR?', os.path.relpath(f, ROOT), i, line.strip()[:110])
