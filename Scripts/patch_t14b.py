# -*- coding: utf-8 -*-
# T14 part 2: enemy ctor yaw + facing via control rotation + FellOutOfWorld,
# GameMode navmesh-projected spawns, NameTagWidget health bar colors.
import io
BASE = 'C:/Users/Blackkus/Documents/Unreal Projects/Salamander/Source/Salamander/'

def patch(path, pairs, append=None):
    p = BASE + path
    s = io.open(p, encoding='utf-8').read()
    for old, new in pairs:
        assert old in s, '%s: anchor not found: %r' % (path, old[:70])
        s = s.replace(old, new, 1)
    if append:
        s += append
    io.open(p, 'w', encoding='utf-8', newline='').write(s)
    print('patched', path)

# --- EnemyCharacter.cpp ---
patch('EnemyCharacter.cpp', [
    ('''AEnemyCharacter::AEnemyCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{''',
     '''AEnemyCharacter::AEnemyCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bUseControllerRotationYaw = true; // the enemy turns to face its target (set from Tick)'''),
    ('''		// Face the target: never walk backwards toward the player.
		const FRotator FaceRot = FRotationMatrix::MakeFromX(Dir).Rotator();
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), FaceRot, DeltaSeconds, 12.f));
		AddMovementInput(Dir, 1.f);''',
     '''		// Face the target: never walk backwards toward the player.
		const FRotator FaceRot = FRotationMatrix::MakeFromX(Dir).Rotator();
		const FRotator NewRot = FMath::RInterpTo(GetActorRotation(), FaceRot, DeltaSeconds, 12.f);
		if (AController* C = GetController())
		{
			C->SetControlRotation(NewRot); // movement input is transformed by control rotation
		}
		SetActorRotation(NewRot);
		AddMovementInput(Dir, 1.f);'''),
    ('''			const FRotator FaceRot = FRotationMatrix::MakeFromX(ToTarget.GetSafeNormal()).Rotator();
			SetActorRotation(FMath::RInterpTo(GetActorRotation(), FaceRot, DeltaSeconds, 12.f));''',
     '''			const FRotator FaceRot = FRotationMatrix::MakeFromX(ToTarget.GetSafeNormal()).Rotator();
			const FRotator NewRot = FMath::RInterpTo(GetActorRotation(), FaceRot, DeltaSeconds, 12.f);
			if (AController* C = GetController())
			{
				C->SetControlRotation(NewRot);
			}
			SetActorRotation(NewRot);'''),
], append='''
void AEnemyCharacter::FellOutOfWorld(const UDamageType& DamageType)
{
	UE_LOG(LogTemp, Warning, TEXT("[Enemy] %s fell out of the world at %s -> destroyed"), *GetName(), *GetActorLocation().ToString());
	Super::FellOutOfWorld(DamageType);
}
''')

# --- SalamanderGameMode.cpp: navmesh-projected spawns + logs ---
patch('SalamanderGameMode.cpp', [
    ('#include "SalamanderInputLib.h"\n',
     '#include "SalamanderInputLib.h"\n#include "NavigationSystem.h"\n'),
    ('''		FVector BaseLoc = FVector::ZeroVector;
		for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
		{
			BaseLoc = It->GetActorLocation();
			break;
		}
		for (int32 i = 0; i < EnemyCount; i++)
		{
			const FVector Loc = BaseLoc + FVector(250.f * (i + 1), 250.f * i, 0.f);
			GetWorld()->SpawnActor<AEnemyCharacter>(AEnemyCharacter::StaticClass(), Loc, FRotator::ZeroRotator);
		}''',
     '''		FVector BaseLoc = FVector::ZeroVector;
		for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
		{
			BaseLoc = It->GetActorLocation();
			break;
		}
		// Project spawn points onto the navmesh so enemies never spawn off the platform edge.
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
		}'''),
    ('''			const FVector Loc = BaseLoc + FVector(900.f, 500.f, 0.f);
			GetWorld()->SpawnActor<ABossCharacter>(ABossCharacter::StaticClass(), Loc, FRotator::ZeroRotator);''',
     '''			const FVector Loc = ProjectToNav(BaseLoc + FVector(900.f, 500.f, 0.f));
			GetWorld()->SpawnActor<ABossCharacter>(ABossCharacter::StaticClass(), Loc, FRotator::ZeroRotator);
			UE_LOG(LogTemp, Log, TEXT("[GameMode] spawned boss at %s"), *Loc.ToString());'''),
])

# --- NameTagWidget.cpp: white brush, red background (missing HP), green fill (current HP) ---
patch('NameTagWidget.cpp', [
    ('#include "UObject/UObjectGlobals.h"\n',
     '#include "UObject/UObjectGlobals.h"\n#include "Engine/Texture2D.h"\n'),
    ('''	HealthBar = NewObject<UProgressBar>(Box, TEXT("HealthBar"));
	HealthBar->SetPercent(1.f);
	HealthBar->WidgetStyle.BackgroundImage.TintColor = FLinearColor(0.f, 0.f, 0.f, 0.55f);
	HealthBar->WidgetStyle.FillImage.TintColor = FLinearColor(0.15f, 0.85f, 0.2f, 1.f);
	HealthBar->SetFillColorAndOpacity(FLinearColor(0.15f, 0.85f, 0.2f, 1.f));
	Box->AddChild(HealthBar);''',
     '''	HealthBar = NewObject<UProgressBar>(Box, TEXT("HealthBar"));
	HealthBar->SetPercent(1.f);
	// Solid white brush so the tints actually render (default style brushes can be empty in game).
	static UTexture2D* WhiteTex = LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EngineResources/WhiteSquareTexture"));
	FSlateBrush WhiteBrush;
	WhiteBrush.SetResourceObject(WhiteTex);
	WhiteBrush.DrawAs = ESlateDrawEffect::None;
	WhiteBrush.TintColor = FLinearColor::White;
	HealthBar->WidgetStyle.FillImage = WhiteBrush;
	HealthBar->WidgetStyle.BackgroundImage = WhiteBrush;
	// red background = the HP already lost, green fill = current HP
	HealthBar->WidgetStyle.BackgroundImage.TintColor = FLinearColor(0.75f, 0.05f, 0.05f, 0.9f);
	HealthBar->WidgetStyle.FillImage.TintColor = FLinearColor(0.1f, 0.8f, 0.15f, 1.f);
	HealthBar->SetFillColorAndOpacity(FLinearColor(0.1f, 0.8f, 0.15f, 1.f));
	Box->AddChild(HealthBar);'''),
    ('''		// red when low
		const FLinearColor C = Fraction > 0.5f ? FLinearColor(0.15f, 0.85f, 0.2f, 1.f)
			: (Fraction > 0.25f ? FLinearColor(0.95f, 0.75f, 0.1f, 1.f) : FLinearColor(0.9f, 0.1f, 0.1f, 1.f));
		HealthBar->SetFillColorAndOpacity(C);
		HealthBar->WidgetStyle.FillImage.TintColor = C;''',
     '''		// fill stays green (current HP); the red background shows the missing part
		const FLinearColor C = FLinearColor(0.1f, 0.8f, 0.15f, 1.f);
		HealthBar->SetFillColorAndOpacity(C);
		HealthBar->WidgetStyle.FillImage.TintColor = C;'''),
])

print('patch_t14b OK')
