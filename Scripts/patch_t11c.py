# -*- coding: utf-8 -*-
# T11 log fixes: migration logs old version; loaded profile logs XP too.
import io
base = 'C:/Users/Blackkus/Documents/Unreal Projects/Salamander/Source/Salamander/'

p = base + 'FileSaveBackend.cpp'
s = io.open(p, encoding='utf-8').read()
old = '''		Profile->SaveVersion = 2;
		UE_LOG(LogTemp, Log, TEXT("[Save] migrated profile %s from v%d to v2"), *Profile->Name, Profile->SaveVersion);'''
new = '''		const int32 OldVersion = Profile->SaveVersion;
		Profile->SaveVersion = 2;
		UE_LOG(LogTemp, Log, TEXT("[Save] migrated profile %s from v%d to v2"), *Profile->Name, OldVersion);'''
assert old in s, 'migrate anchor'
s = s.replace(old, new, 1)
io.open(p, 'w', encoding='utf-8', newline='').write(s)

p = base + 'SalamanderGameMode.cpp'
s = io.open(p, encoding='utf-8').read()
old = 'UE_LOG(LogTemp, Log, TEXT("[GameMode] Loaded profile for %s: Level=%d"), *PS->PlayerName, PS->Level);'
new = 'UE_LOG(LogTemp, Log, TEXT("[GameMode] Loaded profile for %s: Level=%d XP=%d"), *PS->PlayerName, PS->Level, PS->XP);'
assert old in s, 'load anchor'
s = s.replace(old, new, 1)
io.open(p, 'w', encoding='utf-8', newline='').write(s)

print('patch_t11c OK')
