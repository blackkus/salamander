# -*- coding: utf-8 -*-
# Cree les Input Actions IA_Sprint / IA_Crouch et les mappe dans IMC_Default.
# Lancer via : UnrealEditor-Cmd.exe <projet> -run=pythonscript -script=<ce fichier> -log
import unreal

def get_factory():
    for name in ('InputActionFactoryNew', 'InputActionFactory'):
        f = getattr(unreal, name, None)
        if f is not None:
            return f()
    raise RuntimeError('Aucun InputActionFactory trouve')

def ensure_input_action(path, value_type):
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return unreal.load_asset(path)
    action = unreal.EditorAssetLibrary.create_asset(path, '/Game/Input/Actions', unreal.InputAction, get_factory())
    vt_enum = getattr(unreal, 'InputActionValueType', None) or unreal.EInputActionValueType
    setattr(action, 'value_type' if hasattr(action, 'value_type') else 'ValueType', getattr(vt_enum, value_type))
    unreal.EditorAssetLibrary.save_asset(action)
    return action

def map_key_once(imc, action, key):
    for m in imc.mappings:
        if m.action == action and m.key == key:
            return
    trigger = unreal.InputTriggerPressed()
    imc.map_key(action, key, [], [trigger])

ia_sprint = ensure_input_action('/Game/Input/Actions/IA_Sprint', 'BOOLEAN')
ia_crouch = ensure_input_action('/Game/Input/Actions/IA_Crouch', 'BOOLEAN')

imc = unreal.load_asset('/Game/Input/IMC_Default')
map_key_once(imc, ia_sprint, unreal.Keys.LEFT_SHIFT)
map_key_once(imc, ia_crouch, unreal.Keys.LEFT_CONTROL)
unreal.EditorAssetLibrary.save_asset(imc)

unreal.log('IA_Sprint et IA_Crouch crees et mappees dans IMC_Default (LeftShift / LeftControl)')
