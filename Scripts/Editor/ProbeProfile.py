# -*- coding: utf-8 -*-
import unreal, traceback
try:
    names = [n for n in dir(unreal) if 'Profile' in n]
    unreal.log('profile names in unreal: %s' % names)
    cls = unreal.load_class(None, '/Script/Salamander.PlayerProfile')
    unreal.log('load_class -> %s' % cls)
    if cls:
        unreal.log('has SaveVersion prop: %s' % hasattr(cls, 'SaveVersion'))
        try:
            obj = cls()
            unreal.log('call ok: %s' % obj)
        except Exception as e:
            unreal.log('call failed: %s' % e)
        try:
            obj = unreal.new_object(cls)
            unreal.log('new_object ok: %s' % obj)
        except Exception as e:
            unreal.log('new_object failed: %s' % e)
except Exception:
    unreal.log(traceback.format_exc())
