# -*- coding: utf-8 -*-
# T16: key bindings in a fixed-path ini (Config/KeyBindings.ini) so menu and game always agree;
# debug log of bound keys; enemy/boss spawn delayed 3s so the navmesh is built first.
import io
BASE = 'C:/Users/Blackkus/Documents/Unreal Projects/Salamander/Source/Salamander/'

def patch(path, pairs):
    p = BASE + path
    s = io.open(p, encoding='utf-8').read()
    for old, new in pairs:
        assert old in s, '%s: anchor not found: %r' % (path, old[:70])
        s = s.replace(old, new, 1)
    io.open(p, 'w', encoding='utf-8', newline='').write(s)
    print('patched', path)

# 1) fixed-path ini for bindings
patch('SalamanderInputLib.cpp', [
    ('#define BINDINGS_SECTION TEXT("KeyBindings")',
     '''#define BINDINGS_SECTION TEXT("KeyBindings")
// Fixed-path ini so the menu process and the game process always agree (independent of Windows/WindowsEditor flavor).
static const FString& KeyBindingsIni()
{
	static const FString Path = FPaths::ProjectSavedDir() / TEXT("Config/KeyBindings.ini");
	return Path;
}'''),
    ('''	FString Saved;
	if (GConfig != nullptr && GConfig->GetString(BINDINGS_SECTION, *Slot.Slot.ToString(), Saved, GGameUserSettingsIni) && !Saved.IsEmpty())''',
     '''	FString Saved;
	if (GConfig != nullptr && GConfig->GetString(BINDINGS_SECTION, *Slot.Slot.ToString(), Saved, KeyBindingsIni()) && !Saved.IsEmpty())'''),
    ('''			GConfig->SetString(BINDINGS_SECTION, *SlotName.ToString(), *NewKey.GetFName().ToString(), GGameUserSettingsIni);
			GConfig->Flush(false, GGameUserSettingsIni);''',
     '''			GConfig->SetString(BINDINGS_SECTION, *SlotName.ToString(), *NewKey.GetFName().ToString(), KeyBindingsIni());
			GConfig->Flush(false, KeyBindingsIni());'''),
    ('''	// Saved rebinds.
	for (const FBindSlot& Slot : GetSlots())
	{
		const FKey Bound = GetBoundKey(Slot);
		if (Bound != Slot.DefaultKey)
		{
			RebindSlot(Slot.Slot, Bound);
		}
	}''',
     '''	// Saved rebinds.
	for (const FBindSlot& Slot : GetSlots())
	{
		const FKey Bound = GetBoundKey(Slot);
		UE_LOG(LogTemp, Log, TEXT("[Input] slot %s bound=%s default=%s"), *Slot.Slot.ToString(), *Bound.GetFName().ToString(), *Slot.DefaultKey.GetFName().ToString());
		if (Bound != Slot.DefaultKey)
		{
			RebindSlot(Slot.Slot, Bound);
		}
	}'''),
])
# need FPaths include
s = io.open(BASE + 'SalamanderInputLib.cpp', encoding='utf-8').read()
if '#include "Misc/Paths.h"' not in s:
    s = s.replace('#include "GameFramework/InputSettings.h"', '#include "GameFramework/InputSettings.h"\n#include "Misc/Paths.h"')
    io.open(BASE + 'SalamanderInputLib.cpp', 'w', encoding='utf-8', newline='').write(s)
    print('added Paths include')

# 2) GameMode: delay enemy/boss spawn by 3s so the navmesh is built
patch('SalamanderGameMode.cpp', [
    ('''	// Spawn simple enemies (server only). Count via -EnemyCount=<n> (default 2).
	if (GetWorld()->GetNetMode() != NM_Client)
	{''',
     '''	// Spawn simple enemies (server only). Count via -EnemyCount=<n> (default 2).
	// Delayed a few seconds so the navmesh is built before we pick random navigable points.
	if (GetWorld()->GetNetMode() != NM_Client)
	{
		FTimerHandle SpawnHandle;
		GetWorldTimerManager().SetTimer(SpawnHandle, [this]() { SpawnEnemiesAndBoss(); }, 3.f, false);
	}

	// Periodic save every 60 s.'''),
])

print('patch_t16 part1 OK')
