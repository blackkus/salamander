# -*- coding: utf-8 -*-
# Creates the IA_Sprint / IA_Crouch Input Actions. En 5.8, value_type is read-only:
# it is set by the factory at creation (BOOLEAN by default for InputAction_Factory).
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
unreal.log('IA_Sprint and IA_Crouch OK in /Game/Input/Actions')
