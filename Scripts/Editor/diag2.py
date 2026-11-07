# -*- coding: utf-8 -*-
import unreal
def p(*a):
    try:
        unreal.log('DIAG2 ' + ' '.join(str(x) for x in a))
    except Exception as e:
        unreal.log('DIAG2 ERR ' + str(e))

try:
    p('KeysLike:', [m for m in dir(unreal) if 'Keys' in m][:10])
    k = getattr(unreal, 'Keys', None)
    p('Keys exists:', k is not None)
    if k is not None:
        p('LEFT_SHIFT:', hasattr(k, 'LEFT_SHIFT'), 'LEFT_CONTROL:', hasattr(k, 'LEFT_CONTROL'))
    p('factories input/action:', [m for m in dir(unreal) if 'actory' in m and ('nput' in m or 'ction' in m)])
    p('Factory base:', hasattr(unreal, 'Factory'))
    p('InputTriggerPressed:', hasattr(unreal, 'InputTriggerPressed'))
    p('MEL:', [m for m in dir(unreal.MaterialEditingLibrary) if 'creat' in m.lower() or 'connect' in m.lower() or 'recompil' in m.lower()])
    p('factories M/L:', hasattr(unreal, 'MaterialFactoryNew'), hasattr(unreal, 'LevelFactory'))
    p('ELL:', hasattr(unreal.EditorLevelLibrary, 'spawn_actor_from_class'), hasattr(unreal.EditorLevelLibrary, 'load_level'), hasattr(unreal.EditorLevelLibrary, 'save_current_level'))
    p('actors:', hasattr(unreal, 'StaticMeshActor'), hasattr(unreal, 'PlayerStart'), hasattr(unreal, 'DirectionalLight'), hasattr(unreal, 'SkyLight'), hasattr(unreal, 'SkyAtmosphere'))
    p('misc:', hasattr(unreal, 'ComponentMobility'), hasattr(unreal, 'MaterialExpressionVectorParameter'), hasattr(unreal, 'MaterialExpressionConstant3Vector'), hasattr(unreal.MaterialProperty, 'MP_BASE_COLOR'), hasattr(unreal.EditorAssetLibrary, 'save_asset'), hasattr(unreal.EditorAssetLibrary, 'does_asset_exist'))
    at = unreal.AssetToolsHelpers.get_asset_tools()
    p('asset_tools.create_asset sig ok:', callable(getattr(at, 'create_asset', None)))
except Exception as e:
    import traceback
    unreal.log('DIAG2 FATAL ' + traceback.format_exc())
