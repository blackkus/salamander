# -*- coding: utf-8 -*-
import io

def patch(path, pairs):
    s = io.open(path, encoding='utf-8').read()
    for old, new in pairs:
        assert old in s, 'NOT FOUND in %s: %r' % (path, old[:60])
        s = s.replace(old, new)
    io.open(path, 'w', encoding='utf-8').write(s)
    print('PATCHED', path)

patch('Source/Salamander/SalamanderGameMode.h', [(
"""	// Player death (HP = 0): destroys the pawn and respawns at the start point after 3 s.
	void HandlePlayerDeath(class APawn* Pawn);""",
"""	// Player death (HP = 0): destroys the pawn and respawns at the start point after 3 s.
	void HandlePlayerDeath(class APawn* Pawn);

	// Boss killed: gives the reward (DT_BossReward) to every connected player and saves profiles.
	void OnBossKilled(class ABossCharacter* Boss);"""
)])

patch('Source/Salamander/SalamanderGameMode.cpp', [
('#include "EnemyCharacter.h"',
 '#include "EnemyCharacter.h"\n#include "BossCharacter.h"\n#include "RewardTypes.h"\n#include "Engine/DataTable.h"'),
# load XP from profile in Login
("""			if (SaveBackend->LoadProfile(PS->PlayerName, Loaded))
			{
				PS->Level = Loaded->Level;
				PS->ColorRgb = Loaded->ColorRgb;""",
 """			if (SaveBackend->LoadProfile(PS->PlayerName, Loaded))
			{
				PS->Level = Loaded->Level;
				PS->XP = Loaded->XP;
				PS->ColorRgb = Loaded->ColorRgb;"""),
# save XP in SaveProfileFor
("""	Profile->Name = PS->PlayerName;
	Profile->ColorRgb = PS->ColorRgb;
	Profile->Level = PS->Level;""",
 """	Profile->Name = PS->PlayerName;
	Profile->ColorRgb = PS->ColorRgb;
	Profile->Level = PS->Level;
	Profile->XP = PS->XP;"""),
# spawn boss after enemies
("""		UE_LOG(LogTemp, Log, TEXT("[GameMode] spawned %d enemies"), EnemyCount);
	}""",
 """		UE_LOG(LogTemp, Log, TEXT("[GameMode] spawned %d enemies"), EnemyCount);

		// One boss further away (-BossCount=<n>, default 1).
		int32 BossCount = 1;
		FParse::Value(FCommandLine::Get(), TEXT("BossCount="), BossCount);
		for (int32 i = 0; i < BossCount; i++)
		{
			const FVector Loc = BaseLoc + FVector(900.f, 500.f, 0.f);
			GetWorld()->SpawnActor<ABossCharacter>(ABossCharacter::StaticClass(), Loc, FRotator::ZeroRotator);
		}
		if (BossCount > 0)
		{
			UE_LOG(LogTemp, Log, TEXT("[GameMode] spawned %d boss"), BossCount);
		}
	}"""),
# TestAutoBossDamage in PostLogin before AutoLevel block
("""	int32 AutoLevel = 0;
	if (FParse::Value(FCommandLine::Get(), TEXT("TestAutoLevel="), AutoLevel) && AutoLevel > 0)""",
 """	// CLI test: damage the boss from the server (-TestAutoBossDamage=<n>).
	int32 BossDamage = 0;
	if (FParse::Value(FCommandLine::Get(), TEXT("TestAutoBossDamage="), BossDamage) && BossDamage > 0)
	{
		const int32 Amount = BossDamage;
		FTimerHandle BH;
		GetWorldTimerManager().SetTimer(BH, [this, Amount]()
		{
			for (TActorIterator<ABossCharacter> It(GetWorld()); It; ++It)
			{
				if (UAbilitySystemComponent* BossASC = It->AbilitySystemComponent)
				{
					UGE_Damage::ApplyTo(BossASC, static_cast<float>(Amount));
					UE_LOG(LogTemp, Log, TEXT("[GameMode] TestAutoBossDamage %d on %s"), Amount, *It->GetName());
				}
			}
		}, 12.f, false);
	}

	int32 AutoLevel = 0;
	if (FParse::Value(FCommandLine::Get(), TEXT("TestAutoLevel="), AutoLevel) && AutoLevel > 0)"""),
])

# append OnBossKilled at end of GameMode.cpp
s = io.open('Source/Salamander/SalamanderGameMode.cpp', encoding='utf-8').read()
s += """

void ASalamanderGameMode::OnBossKilled(ABossCharacter* Boss)
{
	UE_LOG(LogTemp, Log, TEXT("[GameMode] boss %s killed -> reward to all players"), Boss ? *Boss->GetName() : TEXT("?"));

	// Reward from the Data Table DT_BossReward (row "Boss1").
	UDataTable* RewardTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/DT_BossReward"));
	const FRewardRow* Row = RewardTable != nullptr ? RewardTable->FindRow<FRewardRow>(FName("Boss1"), TEXT("BossReward")) : nullptr;
	const int32 XPReward = Row ? Row->XP : 100;      // PROVISOIRE fallback
	const FString ItemName = Row ? Row->ItemName : TEXT("FictifSword"); // PROVISOIRE fallback

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ASalamanderPlayerState* PS = (*It)->GetPlayerState<ASalamanderPlayerState>())
		{
			PS->XP += XPReward;
			SaveProfileFor(PS); // XP recorded in the saved profile
			UE_LOG(LogTemp, Log, TEXT("[GameMode] reward: %s +%d XP (item %s), total XP=%d"), *PS->PlayerName, XPReward, *ItemName, PS->XP);
		}
	}
}
"""
io.open('Source/Salamander/SalamanderGameMode.cpp', 'w', encoding='utf-8').write(s)
print('GM_APPEND_OK')

# MaterialLib.h: boss ensure functions
patch('Source/Salamander/SalamanderMaterialLib.h', [
('#include "DA_EnemyStats.h"', '#include "DA_EnemyStats.h"\n#include "DA_BossStats.h"'),
("\tstatic UDA_EnemyStats* EnsureEnemyStatsAsset();\n#endif",
 "\tstatic UDA_EnemyStats* EnsureEnemyStatsAsset();\n\n\t// Creates /Game/Data/DA_BossStats if missing (PROVISOIRE values).\n\tUFUNCTION(BlueprintCallable, Category=\"Salamander|Editor\")\n\tstatic UDA_BossStats* EnsureBossStatsAsset();\n\n\t// Creates /Game/Data/DT_BossReward (DataTable, row Boss1) if missing.\n\tUFUNCTION(BlueprintCallable, Category=\"Salamander|Editor\")\n\tstatic class UDataTable* EnsureBossRewardTable();\n#endif"),
])

# MaterialLib.cpp: implementations
s = io.open('Source/Salamander/SalamanderMaterialLib.cpp', encoding='utf-8').read()
s = s.replace('#include "DA_DeathRules.h"', '#include "DA_DeathRules.h"\n#include "DA_BossStats.h"\n#include "RewardTypes.h"\n#include "Engine/DataTable.h"')
s += """

#if WITH_EDITOR
UDA_BossStats* USalamanderMaterialLib::EnsureBossStatsAsset()
{
	return EnsureDataAsset<UDA_BossStats>(TEXT("/Game/Data/DA_BossStats"), TEXT("DA_BossStats"));
}

UDataTable* USalamanderMaterialLib::EnsureBossRewardTable()
{
	UDataTable* Table = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/DT_BossReward"));
	if (Table != nullptr)
	{
		return Table;
	}
	UPackage* Pkg = CreatePackage(TEXT("/Game/Data"));
	Table = NewObject<UDataTable>(Pkg, UDataTable::StaticClass(), TEXT("DT_BossReward"), RF_Public | RF_Standalone);
	Table->RowStruct = FRewardRow::StaticStruct();
	// PROVISOIRE reward row.
	FRewardRow Row;
	Row.XP = 100;
	Row.ItemName = TEXT("FictifSword");
	Table->AddRow(FName("Boss1"), Row);
	Table->MarkPackageDirty();
	const FString FilePath = FPackageName::LongPackageNameToFilename(TEXT("/Game/Data/DT_BossReward"), FPackageName::GetAssetPackageExtension());
	UPackage::Save(Pkg, Table, *FilePath, FSavePackageArgs());
	return Table;
}
#endif
"""
io.open('Source/Salamander/SalamanderMaterialLib.cpp', 'w', encoding='utf-8').write(s)
print('LIBCPP_OK')
