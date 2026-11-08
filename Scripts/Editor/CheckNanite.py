import unreal
m = unreal.load_asset('/Game/Materials/M_GroundGreen')
unreal.log('M_GroundGreen loaded: %s' % m)
for attr in ('nanite_override', 'b_nanite_enabled', 'nanite_enabled'):
    try:
        unreal.log('  %s = %s' % (attr, getattr(m, attr)))
    except Exception as e:
        unreal.log('  %s -> %s' % (attr, e))
# the warning is about FMaterialResource Nanite usage flag; check via static material resource
mr = m.get_material_resource() if hasattr(m, 'get_material_resource') else None
unreal.log('has get_material_resource: %s' % hasattr(m, 'get_material_resource'))
unreal.log('done')
