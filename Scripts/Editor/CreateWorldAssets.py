# -*- coding: utf-8 -*-
# Cree M_PlayerColor (parametre vectoriel BodyColor), M_SolGreen, et la map /Game/Maps/Monde_Test
# avec sol vert, ciel (SkyAtmosphere), soleil, SkyLight et PlayerStart.
import unreal

MATS = '/Game/Materials'

def create_material(path, build_fn):
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return unreal.load_asset(path)
    mat = unreal.EditorAssetLibrary.create_asset(path, MATS, unreal.Material, unreal.MaterialFactoryNew())
    build_fn(mat)
    unreal.MaterialEditingLibrary.recompile_material(mat)
    unreal.EditorAssetLibrary.save_asset(mat)
    return mat

def build_player_color(mat):
    lib = unreal.MaterialEditingLibrary
    node = lib.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, -600, 0)
    node.parameter_name = 'BodyColor'
    node.default_value = unreal.LinearColor(0.8, 0.8, 0.8, 1.0)
    lib.connect_material_property(node, unreal.MaterialProperty.MP_BASE_COLOR)

def build_sol_green(mat):
    lib = unreal.MaterialEditingLibrary
    node = lib.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector, -600, 0)
    node.constant = unreal.LinearColor(0.05, 0.45, 0.08, 1.0)
    lib.connect_material_property(node, unreal.MaterialProperty.MP_BASE_COLOR)

create_material(MATS + '/M_PlayerColor', build_player_color)
create_material(MATS + '/M_SolGreen', build_sol_green)

# --- Map Monde_Test ---
map_path = '/Game/Maps/Monde_Test'
if not unreal.EditorAssetLibrary.does_asset_exist(map_path):
    unreal.EditorAssetLibrary.create_asset(map_path, '/Game/Maps', unreal.Level, unreal.LevelFactory())
unreal.EditorLevelLibrary.load_level(map_path)

floor = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(0, 0, -10), unreal.Rotator())
comp = floor.static_mesh_component
comp.set_mobility(unreal.ComponentMobility.STATIC)
comp.set_static_mesh(unreal.load_asset('/Game/LevelPrototyping/Meshes/SM_Plane'))
comp.set_material(0, unreal.load_asset(MATS + '/M_SolGreen'))
comp.set_world_scale3d(unreal.Vector(200, 200, 1))  # SM_Plane = 1m x 1m -> 200m x 200m

unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(0, 0, 100), unreal.Rotator())
sun = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 2000), unreal.Rotator(-50, -30, 0))
sun.light_component.set_intensity(120000.0)
unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0, 0, 2000), unreal.Rotator())
unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.SkyAtmosphere, unreal.Vector(0, 0, 0), unreal.Rotator())

unreal.EditorLevelLibrary.save_current_level()
unreal.log('Monde_Test cree : sol vert, ciel bleu (SkyAtmosphere), soleil, PlayerStart')
