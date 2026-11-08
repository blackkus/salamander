# -*- coding: utf-8 -*-
# T17: body rotates with mouse (bUseControllerRotationYaw), menu shows the mouse cursor (UIOnly),
# crouch gets a visible mesh squash, input action logs for debugging.
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

# 1) Character: body follows camera yaw + input logs + visible crouch
patch('SalamanderCharacter.cpp', [
    ('''	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;''',
     '''	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true; // the body turns with the mouse (camera boom follows control rotation)
	bUseControllerRotationRoll = false;'''),
    ('''void ASalamanderCharacter::StartSprint()
{
	if (IsDead())
	{
		return;
	}''',
     '''void ASalamanderCharacter::StartSprint()
{
	UE_LOG(LogTemp, Log, TEXT("[Input] sprint ON"));
	if (IsDead())
	{
		return;
	}'''),
    ('''void ASalamanderCharacter::StopSprint()
{
	if (IsDead())
	{
		return;
	}''',
     '''void ASalamanderCharacter::StopSprint()
{
	UE_LOG(LogTemp, Log, TEXT("[Input] sprint OFF"));
	if (IsDead())
	{
		return;
	}'''),
    ('''void ASalamanderCharacter::StartCrouch()
{
	if (IsDead())
	{
		return;
	}
	Crouch();
}''',
     '''void ASalamanderCharacter::StartCrouch()
{
	UE_LOG(LogTemp, Log, TEXT("[Input] crouch ON"));
	if (IsDead())
	{
		return;
	}
	Crouch();
}'''),
    ('''void ASalamanderCharacter::StopCrouch()
{
	if (IsDead())
	{
		return;
	}''',
     '''void ASalamanderCharacter::StopCrouch()
{
	UE_LOG(LogTemp, Log, TEXT("[Input] crouch OFF"));
	if (IsDead())
	{
		return;
	}'''),
    # visible crouch: squash the mesh while crouched (no anim asset yet)
    ('''	if (NameTagComponent != nullptr)
	{
		UNameTagWidget::UpdateNameTag(NameTagComponent, GetDisplayName(), GetHealthFraction(), GetWorld());
	}
}''',
     '''	if (NameTagComponent != nullptr)
	{
		UNameTagWidget::UpdateNameTag(NameTagComponent, GetDisplayName(), GetHealthFraction(), GetWorld());
	}
	// Visible crouch feedback (no anim blueprint yet): squash the mesh.
	if (USkeletalMeshComponent* M = GetMesh())
	{
		const float TargetZ = GetCharacterMovement() != nullptr && GetCharacterMovement()->IsCrouching() ? 0.55f : 1.f;
		const float CurZ = M->GetRelativeScale3D().Z;
		M->SetRelativeScale3D(FVector(1.f, 1.f, FMath::FInterpTo(CurZ, TargetZ, DeltaSeconds, 10.f)));
	}
}'''),
])

# 2) GameMode: menu map -> show mouse cursor + UIOnly input mode
patch('SalamanderGameMode.cpp', [
    ('''void ASalamanderGameMode::ShowLoginMenu()
{
	if (GEngine == nullptr || GEngine->GameViewport == nullptr || LoginWidget.IsValid())
	{
		return;
	}''',
     '''void ASalamanderGameMode::ShowLoginMenu()
{
	// The menu needs the mouse cursor (-game hides it by default).
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		PC->bShowMouseCursor = true;
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(LoginWidget.IsValid() ? LoginWidget->TakeWidget() : nullptr);
		PC->SetInputMode(InputMode);
	}
	if (GEngine == nullptr || GEngine->GameViewport == nullptr || LoginWidget.IsValid())
	{
		return;
	}'''),
    ('#include "NavigationSystem.h"\n',
     '#include "NavigationSystem.h"\n#include "GameFramework/PlayerController.h"\n#include "Kismet/GameplayStatics.h"\n'),
])

print('patch_t17 OK')
