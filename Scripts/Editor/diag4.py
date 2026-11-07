# -*- coding: utf-8 -*-
import unreal
def p(*a):
    unreal.log('DIAG4 ' + ' '.join(str(x) for x in a))
try:
    at = unreal.AssetToolsHelpers.get_asset_tools()
    m = at.create_asset('M_Diag', '/Game/Materials', unreal.Material, unreal.MaterialFactoryNew())
    node = unreal.MaterialEditingLibrary.create_material_expression(m, unreal.MaterialExpressionVectorParameter, -600, 0)
    p('vecparam attrs:', [a for a in dir(node) if 'aram' in a.lower() or 'efault' in a.lower()])
    for expr in ("node.parameter_name = 'BodyColor'", "node.ParameterName = 'BodyColor'", "node.set_parameter_name('BodyColor')", "node.set_parameter_name(unreal.Name('BodyColor'))", "node.default_value = unreal.LinearColor(1,0,0,1)", "node.DefaultValue = unreal.LinearColor(1,0,0,1)"):
        try:
            exec(expr); p('OK  ' + expr)
        except Exception as e:
            p('ERR ' + expr + ' -> ' + str(e)[:90])
    n2 = unreal.MaterialEditingLibrary.create_material_expression(m, unreal.MaterialExpressionConstant3Vector, -400, 0)
    p('const3 attrs:', [a for a in dir(n2) if 'onst' in a.lower()])
    try:
        n2.constant = unreal.LinearColor(0, 1, 0, 1); p('OK  n2.constant')
    except Exception as e:
        p('ERR n2.constant -> ' + str(e)[:90])
    p('delete_asset:', hasattr(unreal.EditorAssetLibrary, 'delete_asset'))
    unreal.EditorAssetLibrary.delete_asset('/Game/Materials/M_Diag')
except Exception:
    import traceback
    unreal.log('DIAG4 FATAL ' + traceback.format_exc())
