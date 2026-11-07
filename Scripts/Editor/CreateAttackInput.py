# -*- coding: utf-8 -*-
# Creates the IA_Attack Input Action (left mouse button; key mapping is added in C++).
import unreal

def ensure_input_action(name):
    path = '/Game/Input/Actions/' + name
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return unreal.load_asset(path)
    at = unreal.AssetToolsHelpers.get_asset_tools()
    factory = unreal.InputAction_Factory()
    action = at.create_asset(name, '/Game/Input/Actions', unreal.InputAction, factory)
    unreal.EditorAssetLibrary.save_asset(path)
    return action

ensure_input_action('IA_Attack')
unreal.log('IA_Attack OK in /Game/Input/Actions')
