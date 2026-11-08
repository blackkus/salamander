import unreal
for path in ('/Game/Input/IMC_Default', '/Game/Input/IMC_MouseLook'):
    imc = unreal.load_asset(path)
    print('== %s' % path)
    maps = imc.default_key_mappings.mappings
    print('  count=%d' % len(maps))
    for m in maps:
        try:
            a = m.action.get_name() if m.action else 'None'
            k = str(m.key.get_fname())
            mods = ','.join(str(x) for x in m.modifiers)
            trig = ','.join(x.get_name() for x in m.triggers)
            print('  action=%s key=%s mods=%s trig=%s' % (a, k, mods, trig))
        except Exception as e:
            print('  row err: %s' % e)
