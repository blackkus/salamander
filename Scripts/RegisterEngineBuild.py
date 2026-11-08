# -*- coding: utf-8 -*-
# Registers C:\Games\UE_5.8 under the project's EngineAssociation GUID so that
# double-clicking Salamander.uproject resolves the editor version (no more
# "unknown editor version" / convert prompt).
import winreg

GUID = '{06F3E561-4717-676B-F2C5-0E946A0D7942}'  # EngineAssociation in Salamander.uproject
ENGINE_ROOT = 'C:' + chr(92) + 'Games' + chr(92) + 'UE_5.8'
BS = chr(92)
key_path = 'SOFTWARE' + BS + 'Epic Games' + BS + 'Unreal Engine' + BS + 'Builds' + BS + GUID

for root, name in [(winreg.HKEY_CURRENT_USER, 'HKCU'), (winreg.HKEY_LOCAL_MACHINE, 'HKLM')]:
    try:
        k = winreg.CreateKeyEx(root, key_path, 0, winreg.KEY_SET_VALUE)
        winreg.SetValueEx(k, '', 0, winreg.REG_SZ, ENGINE_ROOT)
        winreg.CloseKey(k)
        print('registered %s -> %s' % (name + BS + key_path, ENGINE_ROOT))
    except PermissionError:
        print('HKLM not writable (ok if HKCU succeeded)')

k = winreg.OpenKey(winreg.HKEY_CURRENT_USER, key_path)
print('verify HKCU default =', winreg.QueryValueEx(k, '')[0])
