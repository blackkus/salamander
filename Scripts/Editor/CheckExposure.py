import unreal
unreal.EditorLevelLibrary.load_level('/Game/Maps/World_Test')
unreal.log('World_Test loaded, exposure cvar = %s' % unreal.SystemLibrary.get_console_variable('r.EyeAdaptation.CachedLightingPreExposure'))
unreal.log('check done')
