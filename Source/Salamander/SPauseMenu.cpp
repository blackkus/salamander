#include "SPauseMenu.h"

#include "SMenuPanel.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

TSharedPtr<SPauseMenu> SPauseMenu::Instance;



void SPauseMenu::Construct(const FArguments& InArgs)
{
	static FSlateFontInfo TitleFont = []()
	{
		FSlateFontInfo F;
		F.Size = 24;
		return F;
	}();

	Panel = SNew(SMenuPanel);
	// Connect from the pause menu: client travel directly (no GameMode on clients).
	Panel->OnConnect = [](const FString& Ip, const FString& Nick, const FString& Color)
	{
		const FString Url = FString::Printf(TEXT("%s?Nick=%s?Color=%s"), *Ip, *Nick, *Color);
		if (GEngine != nullptr)
		{
			if (UWorld* World = GEngine->GetWorld())
			{
				Hide();
				GEngine->SetClientTravel(World, *Url, TRAVEL_Absolute);
			}
		}
	};

	ChildSlot
	[
		SNew(SBorder)
		.Padding(24.f)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(4.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString("PAUSED"))
				.Font(TitleFont)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(8.f)[ Panel.ToSharedRef() ]
			+ SVerticalBox::Slot().AutoHeight().Padding(8.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(4.f)
				[
					SNew(SButton).Text(FText::FromString("Resume")).OnClicked(this, &SPauseMenu::OnResumeClicked)
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(4.f)
				[
					SNew(SButton).Text(FText::FromString("Quit game")).OnClicked(this, &SPauseMenu::OnQuitClicked)
				]
			]
		]
	];
}

void SPauseMenu::Show()
{
	if (Instance.IsValid() || GEngine == nullptr || GEngine->GameViewport == nullptr)
	{
		return;
	}
	Instance = SNew(SPauseMenu);
	GEngine->GameViewport->AddViewportWidgetContent(Instance.ToSharedRef(), 200);
	// UI mode + visible cursor + pause the local player.
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GEngine->GetWorld(), 0))
	{
		PC->bShowMouseCursor = true;
		FInputModeUIOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->SetPause(true);
	}
	UE_LOG(LogTemp, Log, TEXT("[Pause] menu shown"));
}

void SPauseMenu::Hide()
{
	if (!Instance.IsValid())
	{
		return;
	}
	if (GEngine != nullptr && GEngine->GameViewport != nullptr)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(Instance.ToSharedRef());
	}
	Instance.Reset();
	// Back to game: capture mouse, hide cursor, unpause.
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GEngine->GetWorld(), 0))
	{
		PC->SetPause(false);
		PC->bShowMouseCursor = false;
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		if (GEngine->GameViewport != nullptr)
		{
			GEngine->GameViewport->SetMouseCaptureMode(EMouseCaptureMode::CapturePermanently);
		}
	}
	UE_LOG(LogTemp, Log, TEXT("[Pause] menu hidden"));
}

bool SPauseMenu::IsVisible()
{
	return Instance.IsValid();
}

FReply SPauseMenu::OnResumeClicked()
{
	Hide();
	return FReply::Handled();
}

FReply SPauseMenu::OnQuitClicked()
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GEngine->GetWorld(), 0))
	{
		UKismetSystemLibrary::QuitGame(GEngine->GetWorld(), PC, EQuitPreference::Quit, false);
	}
	return FReply::Handled();
}
