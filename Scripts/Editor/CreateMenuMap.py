# -*- coding: utf-8 -*-
# Cree la map /Game/Maps/Menu (vide : ecran de connexion Slate affiche par-dessus).
# Lancer via : UnrealEditor-Cmd.exe <projet> -run=pythonscript -script=<ce fichier> -log -unattended -nullrhi
import unreal

map_path = '/Game/Maps/Menu'
if unreal.EditorAssetLibrary.does_asset_exist(map_path):
    unreal.log('Map Menu existe deja')
else:
    les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    les.new_level(map_path)
    les.save_current_level()
    unreal.log('Map Menu creee : ' + map_path)
