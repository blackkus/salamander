# -*- coding: utf-8 -*-
# Creates the /Game/Maps/Menu map (empty: the Slate login screen is shown on top).
# Lancer via : UnrealEditor-Cmd.exe <projet> -run=pythonscript -script=<ce fichier> -log -unattended -nullrhi
import unreal

map_path = '/Game/Maps/Menu'
if unreal.EditorAssetLibrary.does_asset_exist(map_path):
    unreal.log('Menu map already exists')
else:
    les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    les.new_level(map_path)
    les.save_current_level()
    unreal.log('Menu map created: ' + map_path)
