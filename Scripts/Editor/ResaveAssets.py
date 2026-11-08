# -*- coding: utf-8 -*-
# Re-saves project assets so post-conversion usage flags (e.g. Nanite) are serialized.
import unreal
assets = [
    '/Game/Materials/M_GroundGreen',
    '/Game/Materials/M_PlayerColor',
    '/Game/Maps/World_Test',
    '/Game/Maps/Menu',
    '/Game/Data/DA_BossStats',
    '/Game/Data/DA_DeathRules',
    '/Game/Data/DA_EnemyStats',
    '/Game/Data/DA_MeleeAttackStats',
    '/Game/Data/DT_BossReward',
]
for a in assets:
    obj = unreal.load_asset(a)
    if obj is None:
        unreal.log('MISSING %s' % a)
        continue
    ok = unreal.EditorAssetLibrary.save_asset(a)
    unreal.log('saved %s -> %s' % (a, ok))
unreal.log('resave done')
