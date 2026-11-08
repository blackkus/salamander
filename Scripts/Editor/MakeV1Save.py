# -*- coding: utf-8 -*-
# Creates a fake v1 save (pre-T11 format) to test the v1->v2 migration on load.
import unreal
cls = unreal.load_class(None, '/Script/Salamander.PlayerProfile')
p = unreal.new_object(cls)
p.SaveVersion = 1
p.Name = 'MigrationTest'
p.Level = 3
p.ColorRgb = unreal.LinearColor(0.2, 0.4, 0.6, 1.0)
ok = unreal.GameplayStatics.save_game_to_slot(p, 'MigrationTest', 0)
unreal.log('v1 save written: %s (SaveVersion=%s)' % (ok, p.SaveVersion))
