# -*- coding: utf-8 -*-
# Ensures the Data Assets via C++ editor helpers (one by one, robust).
import unreal, traceback
for fn in ('ensure_death_rules_asset', 'ensure_melee_stats_asset', 'ensure_enemy_stats_asset'):
    try:
        da = getattr(unreal.SalamanderMaterialLib, fn)()
        unreal.log('%s -> %s' % (fn, da.get_name() if da else 'None'))
    except Exception:
        unreal.log('FAILED %s:\n%s' % (fn, traceback.format_exc()))
unreal.log('Stats Data Assets done')
