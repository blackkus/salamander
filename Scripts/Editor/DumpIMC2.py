import unreal
imc = unreal.load_asset('/Game/Input/IMC_Default')
for m in imc.default_key_mappings.mappings:
    try:
        a = m.action.get_name() if m.action else '?'
        k = str(m.key)
        trig = []
        try:
            trig = [t.get_name() for t in m.triggers]
        except Exception:
            trig = ['<err>']
        print('ACT=%s KEY=%s TRIG=%s' % (a, k, trig))
    except Exception as e:
        print('ROWERR %s' % e)
