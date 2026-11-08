# -*- coding: utf-8 -*-
# Creates M_PlayerColor (BodyColor vector param), M_GroundGreen, and the /Game/Maps/World_Test map
# (green floor, SkyAtmosphere, sun, SkyLight, PlayerStart). Idempotent.
# Lancer via : UnrealEditor-Cmd.exe <projet> -run=pythonscript -script=<ce fichier> -log -unattended -nullrhi
import unreal

MATS = '/Game/Materials'

def create_asset(path, package_path, cls, factory):
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return unreal.load_asset(path)
    at = unreal.AssetToolsHelpers.get_asset_tools()
    return at.create_asset(path.rsplit('/', 1)[-1], package_path, cls, factory)

def build_player_color(mat):
    lib = unreal.MaterialEditingLibrary
    # ParameterName is not exposed in Python 5.8: C++ helper.
    node = unreal.SalamanderMaterialLib.create_vector_parameter_node(mat, 'BodyColor', unreal.LinearColor(0.8, 0.8, 0.8, 1.0))
    lib.connect_material_property(node, '', unreal.MaterialProperty.MP_BASE_COLOR)

def build_sol_green(mat):
    lib = unreal.MaterialEditingLibrary
    node = lib.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector, -600, 0)
    node.constant = unreal.LinearColor(0.05, 0.45, 0.08, 1.0)
    lib.connect_material_property(node, '', unreal.MaterialProperty.MP_BASE_COLOR)

for path, fn in ((MATS + '/M_PlayerColor', build_player_color), (MATS + '/M_GroundGreen', build_sol_green)):
    mat = create_asset(path, MATS, unreal.Material, unreal.MaterialFactoryNew())
    fn(mat)
    unreal.MaterialEditingLibrary.recompile_material(mat)
    unreal.EditorAssetLibrary.save_asset(path)

map_path = '/Game/Maps/World_Test'
les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
if not unreal.EditorAssetLibrary.does_asset_exist(map_path):
    les.new_level(map_path)
else:
    les.load_level(map_path)

acs = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
def spawn(cls, loc, rot):
    if hasattr(acs, 'spawn_actor_from_class'):
        return acs.spawn_actor_from_class(cls, loc, rot)
    return unreal.EditorLevelLibrary.spawn_actor_from_class(cls, loc, rot)

floor = spawn(unreal.StaticMeshActor, unreal.Vector(0, 0, -10), unreal.Rotator())
comp = floor.static_mesh_component
comp.set_mobility(unreal.ComponentMobility.STATIC)
comp.set_static_mesh(unreal.load_asset('/Game/LevelPrototyping/Meshes/SM_Plane'))
comp.set_material(0, unreal.load_asset(MATS + '/M_GroundGreen'))
comp.set_world_scale3d(unreal.Vector(200, 200, 1))

spawn(unreal.PlayerStart, unreal.Vector(0, 0, 100), unreal.Rotator())
sun = spawn(unreal.DirectionalLight, unreal.Vector(0, 0, 2000), unreal.Rotator(-50, -30, 0))
sun.light_component.set_intensity(120000.0)
spawn(unreal.SkyLight, unreal.Vector(0, 0, 2000), unreal.Rotator())
spawn(unreal.SkyAtmosphere, unreal.Vector(0, 0, 0), unreal.Rotator())

les.save_current_level()
unreal.log('World_Test OK: green floor, blue sky, sun, SkyLight, PlayerStart + M_PlayerColor/M_GroundGreen')
