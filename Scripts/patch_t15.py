# -*- coding: utf-8 -*-
# T15: rebind fallback to default key, navmesh random spawn points, mouse capture (GameOnly),
# white texture via ConstructorHelpers (safe at CDO time).
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

# 1) RebindSlot: fall back to the slot's default key when the saved key has no mapping yet
patch('SalamanderInputLib.cpp', [
    ('''		FEnhancedActionKeyMapping OldMapping;
		if (!FindMapping(IMC, Action, OldKey, OldMapping))
		{
			return;
		}
		IMC->UnmapKey(Action, OldKey);''',
     '''		FEnhancedActionKeyMapping OldMapping;
		FKey FromKey = OldKey;
		if (!FindMapping(IMC, Action, FromKey, OldMapping))
		{
			// saved key not mapped yet (fresh process): rebind from the default key instead
			FromKey = Slot.DefaultKey;
			if (!FindMapping(IMC, Action, FromKey, OldMapping))
			{
				return;
			}
		}
		IMC->UnmapKey(Action, FromKey);'''),
    ('''		UE_LOG(LogTemp, Log, TEXT("[Input] rebound %s: %s -> %s"), *SlotName.ToString(), *OldKey.GetFName().ToString(), *NewKey.GetFName().ToString());''',
     '''		UE_LOG(LogTemp, Log, TEXT("[Input] rebound %s: %s -> %s"), *SlotName.ToString(), *FromKey.GetFName().ToString(), *NewKey.GetFName().ToString());'''),
])

# 2) GameMode: spawn enemies/boss on random navmesh points near the player start
patch('SalamanderGameMode.cpp', [
    ('''		// Project spawn points onto the navmesh so enemies never spawn off the platform edge.
		auto ProjectToNav = [this](const FVector& InLoc) -> FVector
		{
			FVector Out = InLoc;
			UNavigationSystemV1::K2_ProjectPointToNavigation(GetWorld(), InLoc, Out, nullptr, nullptr, FVector(400.f));
			return Out;
		};
		for (int32 i = 0; i < EnemyCount; i++)
		{
			const FVector Loc = ProjectToNav(BaseLoc + FVector(250.f * (i + 1), 250.f * i, 0.f));
			GetWorld()->SpawnActor<AEnemyCharacter>(AEnemyCharacter::StaticClass(), Loc, FRotator::ZeroRotator);
			UE_LOG(LogTemp, Log, TEXT("[GameMode] spawned enemy %d at %s"), i, *Loc.ToString());
		}''',
     '''		// Spawn on random NAVIGABLE points near the player start: enemies never spawn off the platform.
		auto RandomNavPoint = [this](float Radius) -> FVector
		{
			FNavLocation Out;
			if (UNavigationSystemV1::GetRandomPointInNavigableRadius(GetWorld(), BaseLoc, Radius, Out))
			{
				return Out.Location;
			}
			return BaseLoc; // no navmesh: stay at the player start
		};
		for (int32 i = 0; i < EnemyCount; i++)
		{
			const FVector Loc = RandomNavPoint(400.f + 150.f * i);
			GetWorld()->SpawnActor<AEnemyCharacter>(AEnemyCharacter::StaticClass(), Loc, FRotator::ZeroRotator);
			UE_LOG(LogTemp, Log, TEXT("[GameMode] spawned enemy %d at %s"), i, *Loc.ToString());
		}'''),
    ('''			const FVector Loc = ProjectToNav(BaseLoc + FVector(900.f, 500.f, 0.f));
			GetWorld()->SpawnActor<ABossCharacter>(ABossCharacter::StaticClass(), Loc, FRotator::ZeroRotator);
			UE_LOG(LogTemp, Log, TEXT("[GameMode] spawned boss at %s"), *Loc.ToString());''',
     '''			const FVector Loc = RandomNavPoint(900.f);
			GetWorld()->SpawnActor<ABossCharacter>(ABossCharacter::StaticClass(), Loc, FRotator::ZeroRotator);
			UE_LOG(LogTemp, Log, TEXT("[GameMode] spawned boss at %s"), *Loc.ToString());'''),
])

# 3) Character: capture the mouse (GameOnly) on the local client so mouse-look deltas flow
patch('SalamanderCharacter.cpp', [
    ('''					USalamanderInputLib::ApplyAllBindings();
					UE_LOG(LogTemp, Log, TEXT("[Char] input contexts added: default=%d mouse=%d"), DefaultMappingContext != nullptr ? 1 : 0, MouseLookContext != nullptr ? 1 : 0);''',
     '''					USalamanderInputLib::ApplyAllBindings();
					UE_LOG(LogTemp, Log, TEXT("[Char] input contexts added: default=%d mouse=%d"), DefaultMappingContext != nullptr ? 1 : 0, MouseLookContext != nullptr ? 1 : 0);

					// Capture the mouse and hide the cursor so mouse-look deltas reach the game.
					PC->bShowMouseCursor = false;
					FInputModeGameOnly InputMode;
					PC->SetInputMode(InputMode);'''),
    ('#include "TimerManager.h"\n',
     '#include "TimerManager.h"\n#include "GameFramework/InputMode.h"\n'),
])

# 4) NameTagWidget: load the white texture with ConstructorHelpers (safe during CDO construction)
patch('NameTagWidget.cpp', [
    ('''	// Solid white brush so the tints actually render (default style brushes can be empty in game).
	static UTexture2D* WhiteTex = LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EngineResources/WhiteSquareTexture"));
	FSlateBrush WhiteBrush;
	WhiteBrush.SetResourceObject(WhiteTex);''',
     '''	// Solid white brush so the tints actually render (default style brushes can be empty in game).
	static ConstructorHelpers::FObjectFinder<UTexture2D> WhiteTexAsset(TEXT("/Engine/EngineResources/WhiteSquareTexture"));
	FSlateBrush WhiteBrush;
	WhiteBrush.SetResourceObject(WhiteTexAsset.Succeeded() ? WhiteTexAsset.Object : nullptr);'''),
])

print('patch_t15 OK')
