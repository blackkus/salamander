import unreal
names = [n for n in dir(unreal) if 'Salamander' in n]
unreal.log('unreal Salamander*: %s' % names)
cls = unreal.load_class(None, '/Script/Salamander.SalamanderInputLib')
unreal.log('cls=%s' % cls)
unreal.log('funcs: %s' % [f.get_name() for f in cls.get_functions()] if cls else 'none')
