# -*- coding: utf-8 -*-
# Cree les Input Actions IA_Sprint / IA_Crouch. En 5.8, value_type est read-only :
# il est fixe par le factory a la creation (BOOLEAN par defaut pour InputAction_Factory).
import unreal

def ensure_input_action(name):
    path = '/Game/Input/Actions/' + name
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        action = unreal.load_asset(path)
    else:
        at = unreal.AssetToolsHelpers.get_asset_tools()
        factory = unreal.InputAction_Factory()
        action = at.create_asset(name, '/Game/Input/Actions', unreal.InputAction, factory)
        unreal.EditorAssetLibrary.save_asset(path)
    unreal.log('InputAction %s value_type=%s' % (name, action.value_type))
    return action

ensure_input_action('IA_Sprint')
ensure_input_action('IA_Crouch')
unreal.log('IA_Sprint et IA_Crouch OK dans /Game/Input/Actions')
