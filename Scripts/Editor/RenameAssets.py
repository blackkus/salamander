# -*- coding: utf-8 -*-
# Renames assets through EditorAssetLibrary.rename_asset so references are fixed up.
import unreal, traceback
renames = [
    ('/Game/Maps/Monde_Test', '/Game/Maps/World_Test'),
    ('/Game/Materials/M_SolGreen', '/Game/Materials/M_GroundGreen'),
]
for old, new in renames:
    try:
        ok = unreal.EditorAssetLibrary.rename_asset(old, new)
        unreal.log('rename %s -> %s : %s' % (old, new, ok))
    except Exception:
        unreal.log('FAILED %s -> %s:\n%s' % (old, new, traceback.format_exc()))
unreal.log('renames done')
