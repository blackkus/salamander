# -*- coding: utf-8 -*-
# T14 fixes: mouse-button rebind capture, enemy facing via control rotation + FellOutOfWorld log,
# navmesh-projected enemy spawns, health bar red background / green fill with a real white brush,
# diagnostic log for input contexts.
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

# 1) SLoginWidget: capture mouse buttons too (attack = LMB rebind)
patch('SLoginWidget.h', [
    ('''	// Captures the next pressed key while a rebind is pending.
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;''',
     '''	// Captures the next pressed key while a rebind is pending.
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	// Mouse buttons count as keys too (attack = left mouse button).
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& InPointerEvent) override;'''),
])
patch('SLoginWidget.cpp', [
    ('''FReply SLoginWidget::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (PendingSlot != NAME_None)
	{
		const FKey NewKey = InKeyEvent.GetKey();
		USalamanderInputLib::RebindSlot(PendingSlot, NewKey);
		PendingSlot = NAME_None;
		if (HintText.IsValid())
		{
			HintText->SetText(FText::FromString(TEXT("")));
		}
		RefreshSlotButtons();
		return FReply::Handled();
	}
	return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}''',
     '''FReply SLoginWidget::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (PendingSlot != NAME_None)
	{
		const FKey NewKey = InKeyEvent.GetKey();
		USalamanderInputLib::RebindSlot(PendingSlot, NewKey);
		PendingSlot = NAME_None;
		if (HintText.IsValid())
		{
			HintText->SetText(FText::FromString(TEXT("")));
		}
		RefreshSlotButtons();
		return FReply::Handled();
	}
	return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}

FReply SLoginWidget::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& InPointerEvent)
{
	if (PendingSlot != NAME_None)
	{
		USalamanderInputLib::RebindSlot(PendingSlot, InPointerEvent.GetEffectiveButton());
		PendingSlot = NAME_None;
		if (HintText.IsValid())
		{
			HintText->SetText(FText::FromString(TEXT("")));
		}
		RefreshSlotButtons();
		return FReply::Handled();
	}
	return SCompoundWidget::OnMouseButtonDown(MyGeometry, InPointerEvent);
}'''),
])

# 2) SalamanderCharacter: log input contexts
patch('SalamanderCharacter.cpp', [
    ('''					// Saved key bindings + AZERTY alt keys (WASD/ZQSD).
					USalamanderInputLib::ApplyAllBindings();
''',
     '''					// Saved key bindings + AZERTY alt keys (WASD/ZQSD).
					USalamanderInputLib::ApplyAllBindings();

					UE_LOG(LogTemp, Log, TEXT("[Char] input contexts: default=%d mouse=%d"), DefaultMappingContext != nullptr ? 1 : 0, MouseLookContext != nullptr ? 1 : 0);
'''),
])

# 3) Enemy: face the player via control rotation; log when falling out of the world
patch('EnemyCharacter.h', [
    ('class UWidgetComponent;\n',
     'class UWidgetComponent;\nclass UDamageType;\n'),
    ('''	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
''',
     '''	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void FellOutOfWorld(const UDamageType& DamageType) override;
'''),
])
patch('EnemyCharacter.cpp', [
    ('#include "NameTagWidget.h"\n',
     '#include "NameTagWidget.h"\n#include "GameFramework/DamageType.h"\n#include "GameFramework/Controller.h"\n'),
    ('''AEnemyCharacter::AEnemyCharacter(const FObjectInitializer& ObjectInitializer)''',
     '''AEnemyCharacter::AEnemyCharacter(const FObjectInitializer& ObjectInitializer)'''),
])
# ctor body: set bUseControllerRotationYaw — find the ctor body start
s = io.open(BASE + 'EnemyCharacter.cpp', encoding='utf-8').read()
anchor = 'AEnemyCharacter::AEnemyCharacter(const FObjectInitializer& ObjectInitializer)\n{'
assert anchor in s
s = s.replace(anchor, anchor + '\n\tbUseControllerRotationYaw = true; // the enemy turns to face its target (set from Tick)\n', 1)
# facing: replace both SetActorRotation interp blocks with control+actor rotation
old_chase = '''		// Face the target: never walk backwards toward the player.
		const FRotator FaceRot = FRotationMatrix::MakeFromX(Dir).Rotator();
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), FaceRot, DeltaSeconds, 12.f));
		AddMovementInput(Dir, 1.f);'''
new_chase = '''		// Face the target: never walk backwards toward the player.
		const FRotator FaceRot = FRotationMatrix::MakeFromX(Dir).Rotator();
		const FRotator NewRot = FMath::RInterpTo(GetActorRotation(), FaceRot, DeltaSeconds, 12.f);
		if (AController* C = GetController())
		{
			C->SetControlRotation(NewRot); // movement input is transformed by control rotation
		}
		SetActorRotation(NewRot);
		AddMovementInput(Dir, 1.f);'''
assert old_chase in s
s = s.replace(old_chase, new_chase, 1)
old_warn = '''			const FRotator FaceRot = FRotationMatrix::MakeFromX(ToTarget.GetSafeNormal()).Rotator();
			SetActorRotation(FMath::RInterpTo(GetActorRotation(), FaceRot, DeltaSeconds, 12.f));'''
new_warn = '''			const FRotator FaceRot = FRotationMatrix::MakeFromX(ToTarget.GetSafeNormal()).Rotator();
			const FRotator NewRot = FMath::RInterpTo(GetActorRotation(), FaceRot, DeltaSeconds, 12.f);
			if (AController* C = GetController())
			{
				C->SetControlRotation(NewRot);
			}
			SetActorRotation(NewRot);'''
assert old_warn in s
s = s.replace(old_warn, new_warn, 1)
# FellOutOfWorld impl at end of file
s += '''
void AEnemyCharacter::FellOutOfWorld(const UDamageType& DamageType)
{
	UE_LOG(LogTemp, Warning, TEXT("[Enemy] %s fell out of the world at %s -> destroyed"), *GetName(), *GetActorLocation().ToString());
	Super::FellOutOfWorld(DamageType);
}
'''
io.open(BASE + 'EnemyCharacter.cpp', 'w', encoding='utf-8', newline='').write(s)
print('patched EnemyCharacter.cpp')

# 4) GameMode: project enemy/boss spawns onto the navmesh + log spawn locations
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

# 5) NameTagWidget: real white brush; background red (missing HP), fill green (current HP)
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

print('patch_t14 OK')
