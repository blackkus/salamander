# -*- coding: utf-8 -*-
import unreal
def p(*a): unreal.log('DIAG6 ' + ' '.join(str(x) for x in a))
try:
    for expr in ("unreal.load_class(None, '/Script/Salamander.DA_DeathRules')",
                 "unreal.load_class(unreal.load_asset('/Game/Maps/Menu'), '/Script/Salamander.DA_DeathRules')",
                 "unreal.find_class('DA_DeathRules') if hasattr(unreal, 'find_class') else 'no find_class'",
                 "getattr(unreal, 'DA_DeathRules', None)"):
        try:
            c = eval(expr)
            p('OK ', expr, '->', c)
        except Exception as e:
            p('ERR', expr, '->', str(e)[:90])
except Exception:
    import traceback
    unreal.log('DIAG6 FATAL ' + traceback.format_exc())
