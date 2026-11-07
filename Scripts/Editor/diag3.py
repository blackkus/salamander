# -*- coding: utf-8 -*-
import unreal
def p(*a):
    unreal.log('DIAG3 ' + ' '.join(str(x) for x in a))
try:
    p('Key members:', [m for m in dir(unreal) if m in ('Key', 'Keys', 'EKeys', 'FKey')])
    p('enums with Key:', [m for m in dir(unreal) if 'Key' in m and not m.startswith('_')][:25])
    for expr in ("unreal.Key('LeftShift')", "unreal.Key(unreal.Name('LeftShift'))"):
        try:
            k = eval(expr)
            p(expr, '->', repr(k), 'name:', getattr(k, 'key_name', None))
        except Exception as e:
            p(expr, 'ERR', str(e))
    # via InputMappingContext map_key avec FKey depuis string?
    imc = unreal.load_asset('/Game/Input/IMC_Default')
    ia = unreal.load_asset('/Game/Input/Actions/IA_Move')
    p('existing mapping sample:', [(str(m.key), str(m.action)) for m in list(imc.mappings)[:3]])
except Exception:
    import traceback
    unreal.log('DIAG3 FATAL ' + traceback.format_exc())
