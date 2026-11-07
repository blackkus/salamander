# -*- coding: utf-8 -*-
import unreal
def p(*a):
    unreal.log('DIAG5 ' + ' '.join(str(x) for x in a))
try:
    les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    p('LES level/save/new methods:', [m for m in dir(les) if 'level' in m.lower() or 'save' in m.lower() or m.startswith('new')])
    p('spawn/actor methods:', [m for m in dir(les) if 'spawn' in m.lower() or 'actor' in m.lower()])
    p('has new_level:', hasattr(les, 'new_level'), 'load_level:', hasattr(les, 'load_level'), 'save_current_level:', hasattr(les, 'save_current_level'))
except Exception:
    import traceback
    unreal.log('DIAG5 FATAL ' + traceback.format_exc())
