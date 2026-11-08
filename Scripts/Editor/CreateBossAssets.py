# -*- coding: utf-8 -*-
# Ensures DA_BossStats + DT_BossReward via C++ editor helpers.
import unreal, traceback
for fn in ('ensure_boss_stats_asset', 'ensure_boss_reward_table'):
    try:
        obj = getattr(unreal.SalamanderMaterialLib, fn)()
        unreal.log('%s -> %s' % (fn, obj.get_name() if obj else 'None'))
    except Exception:
        unreal.log('FAILED %s:\n%s' % (fn, traceback.format_exc()))
unreal.log('Boss assets done')
