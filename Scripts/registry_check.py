# -*- coding: utf-8 -*-
# Lists Epic Unreal Engine registrations in the registry (Builds / Identifiers).
import winreg

SUBS = [
    r'SOFTWARE\Epic Games\Unreal Engine\Builds',
    r'SOFTWARE\WOW6432Node\Epic Games\Unreal Engine\Builds',
    r'SOFTWARE\Epic Games\Unreal Engine\Identifiers',
    r'SOFTWARE\WOW6432Node\Epic Games\Unreal Engine\Identifiers',
]

for root, name in [(winreg.HKEY_LOCAL_MACHINE, 'HKLM'), (winreg.HKEY_CURRENT_USER, 'HKCU')]:
    for sub in SUBS:
        try:
            k = winreg.OpenKey(root, sub)
        except FileNotFoundError:
            continue
        nvals, nkeys, _ = winreg.QueryInfoKey(k)
        print('%s\\%s : %d values, %d subkeys' % (name, sub, nvals, nkeys))
        for j in range(nvals):
            vn, vv, _ = winreg.EnumValue(k, j)
            print('   value %r = %r' % (vn, str(vv)[:100]))
        for i in range(nkeys):
            sk = winreg.EnumKey(k, i)
            kk = winreg.OpenKey(root, sub + '\\' + sk)
            nv = winreg.QueryInfoKey(kk)[0]
            print('   subkey %r:' % sk)
            for j in range(nv):
                vn, vv, _ = winreg.EnumValue(kk, j)
                print('      %r = %r' % (vn, str(vv)[:100]))
