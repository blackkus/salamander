# -*- coding: utf-8 -*-
# T18: sprint 6000, bigger crouch squash, permanent mouse capture in game, crosshair widget on game map.
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

# 1) sprint speed 1000 -> 6000 (PLACEHOLDER value)
patch('SalamanderCharacterMovementComponent.h', [
    ('float SprintMaxWalkSpeed = 1000.f;', 'float SprintMaxWalkSpeed = 6000.f; // PLACEHOLDER, very visible for tests'),
])

# 2) bigger crouch squash + permanent mouse capture in game
patch('SalamanderCharacter.cpp', [
    ('const float TargetZ = GetCharacterMovement() != nullptr && GetCharacterMovement()->IsCrouching() ? 0.55f : 1.f;',
     'const float TargetZ = GetCharacterMovement() != nullptr && GetCharacterMovement()->IsCrouching() ? 0.35f : 1.f;'),
    ('''					// Capture the mouse and hide the cursor so mouse-look deltas reach the game.
					PC->bShowMouseCursor = false;
					FInputModeGameOnly InputMode;
					PC->SetInputMode(InputMode);''',
     '''					// Capture the mouse permanently and hide the cursor so mouse-look deltas reach the game.
					PC->bShowMouseCursor = false;
					FInputModeGameOnly InputMode;
					PC->SetInputMode(InputMode);
					if (GEngine != nullptr && GEngine->GameViewport != nullptr)
					{
						GEngine->GameViewport->SetMouseCaptureMode(EMouseCaptureMode::CapturePermanently);
					}'''),
])

# 3) GameMode: crosshair on the game map, hidden in menu
patch('SalamanderGameMode.h', [
    ('''	void ShowLoginMenu();''',
     '''	void ShowLoginMenu();
	void ShowCrosshair();
	void HideCrosshair();'''),
])
s = io.open(BASE + 'SalamanderGameMode.h', encoding='utf-8').read()
if 'CrosshairWidget' not in s:
    s = s.replace('void HideLoginMenu();', 'void HideLoginMenu();\n\n\tTSharedPtr<SWidget> CrosshairWidget;')
    io.open(BASE + 'SalamanderGameMode.h', 'w', encoding='utf-8', newline='').write(s)
    print('patched SalamanderGameMode.h (member)')

patch('SalamanderGameMode.cpp', [
    ('#include "Kismet/GameplayStatics.h"\n',
     '#include "Kismet/GameplayStatics.h"\n#include "Widgets/Text/STextBlock.h"\n#include "Widgets/Layout/SBox.h"\n'),
    ('''		DefaultPawnClass = nullptr;
		USalamanderInputLib::ApplyAllBindings();
		ShowLoginMenu();''',
     '''		DefaultPawnClass = nullptr;
		USalamanderInputLib::ApplyAllBindings();
		ShowLoginMenu();
		HideCrosshair();'''),
    ('''	// Menu map (client started without IP): no pawn, show the Slate login screen.''',
     '''	// Game map (client): show a crosshair; the OS cursor is hidden (GameOnly input mode).
	if (GetWorld()->GetNetMode() != NM_DedicatedServer && !MapName.Contains(TEXT("Menu")))
	{
		ShowCrosshair();
	}

	// Menu map (client started without IP): no pawn, show the Slate login screen.'''),
])

# append crosshair functions at end of GameMode cpp
s = io.open(BASE + 'SalamanderGameMode.cpp', encoding='utf-8').read()
s = s.rstrip() + '''

void ASalamanderGameMode::ShowCrosshair()
{
	if (GEngine == nullptr || GEngine->GameViewport == nullptr || CrosshairWidget.IsValid())
	{
		return;
	}
	CrosshairWidget = SNew(SBox)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("+")))
			.ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 0.9f))
			.ShadowOffset(FVector2D(1.f, 1.f))
			.ShadowColorAndOpacity(FLinearColor::Black)
		];
	FSlateFontInfo F = FSlateFontInfo();
	F.Size = 28;
	StaticCastSharedPtr<STextBlock>(CrosshairWidget)->SetFont(F);
	GEngine->GameViewport->AddViewportWidgetContent(CrosshairWidget.ToSharedRef(), 50);
	UE_LOG(LogTemp, Log, TEXT("[GameMode] crosshair shown"));
}

void ASalamanderGameMode::HideCrosshair()
{
	if (GEngine != nullptr && GEngine->GameViewport != nullptr && CrosshairWidget.IsValid())
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(CrosshairWidget.ToSharedRef());
		CrosshairWidget.Reset();
	}
}
'''
io.open(BASE + 'SalamanderGameMode.cpp', 'w', encoding='utf-8', newline='').write(s)
print('patched SalamanderGameMode.cpp (crosshair)')

print('patch_t18 OK')
