# -*- coding: utf-8 -*-
# Creates /Game/Data/DA_DeathRules via the C++ editor helper (reliable).
import unreal
da = unreal.SalamanderMaterialLib.ensure_death_rules_asset()
unreal.log('DA_DeathRules ensured: class=%s RespawnDelay=%.1f (PLACEHOLDER)' % (da.get_class().get_name(), da.respawn_delay))
