# -*- coding: utf-8 -*-
# Points World_Test's floor materials to M_GroundGreen, saves the map, deletes the old M_SolGreen.
import unreal, traceback
try:
    unreal.EditorLevelLibrary.load_level('/Game/Maps/World_Test')
    new_mat = unreal.load_asset('/Game/Materials/M_GroundGreen')
    fixed = 0
    for actor in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
        for comp in actor.get_components_by_class(unreal.PrimitiveComponent):
            for i in range(comp.get_num_materials()):
                m = comp.get_material(i)
                if m and 'M_SolGreen' in m.get_path_name():
                    comp.set_material(i, new_mat)
                    fixed += 1
    unreal.log('material slots fixed: %d' % fixed)
    unreal.EditorLevelLibrary.save_current_level()
    ok = unreal.EditorAssetLibrary.delete_asset('/Game/Materials/M_SolGreen')
    unreal.log('delete old M_SolGreen: %s' % ok)
    unreal.log('fix done')
except Exception:
    unreal.log(traceback.format_exc())
