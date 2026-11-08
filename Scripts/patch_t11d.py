# -*- coding: utf-8 -*-
# T11: server CLI hook -TestMigrateV1=<key> to prove v1->v2 migration in LoadProfile.
import io
base = 'C:/Users/Blackkus/Documents/Unreal Projects/Salamander/Source/Salamander/'
p = base + 'SalamanderGameMode.cpp'
s = io.open(p, encoding='utf-8').read()
anchor = '''	// Periodic save every 60 s.
	GetWorldTimerManager().SetTimer(SaveTimerHandle, this, &ASalamanderGameMode::SaveAllProfiles, 60.f, true);'''
add = anchor + '''

	// CLI test: -TestMigrateV1=<key> writes a v1 profile, then loads it back through the save backend.
	FString MigrateKey;
	if (FParse::Value(FCommandLine::Get(), TEXT("TestMigrateV1="), MigrateKey) && !MigrateKey.IsEmpty() && SaveBackend != nullptr)
	{
		UPlayerProfile* V1 = NewObject<UPlayerProfile>(this);
		V1->SaveVersion = 1;
		V1->Name = MigrateKey;
		V1->Level = 7;
		V1->ColorRgb = FLinearColor(0.1f, 0.2f, 0.3f, 1.f);
		SaveBackend->SaveProfile(MigrateKey, V1);
		UPlayerProfile* Loaded = NewObject<UPlayerProfile>(this);
		if (SaveBackend->LoadProfile(MigrateKey, Loaded))
		{
			UE_LOG(LogTemp, Log, TEXT("[GameMode] TestMigrateV1 %s -> SaveVersion=%d Level=%d XP=%d"), *MigrateKey, Loaded->SaveVersion, Loaded->Level, Loaded->XP);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[GameMode] TestMigrateV1 %s load failed"), *MigrateKey);
		}
	}'''
assert anchor in s, 'anchor not found'
s = s.replace(anchor, add, 1)
io.open(p, 'w', encoding='utf-8', newline='').write(s)
print('patch_t11d OK')
