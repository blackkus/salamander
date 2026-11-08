# -*- coding: utf-8 -*-
import unreal, traceback
d = unreal.AssetRenameData()
unreal.log('AssetRenameData attrs: %s' % [a for a in dir(d) if not a.startswith('_')])
unreal.log('EditorAssetLibrary has rename_asset: %s' % hasattr(unreal.EditorAssetLibrary, 'rename_asset'))
unreal.log('AssetTools methods: %s' % [a for a in dir(unreal.AssetTools) if 'rename' in a.lower()])
