#include "NameTagWidget.h"

#include "Blueprint/WidgetTree.h"
#include "UObject/UObjectGlobals.h"
#include "Engine/Texture2D.h"

#include "Components/WidgetComponent.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"

UNameTagWidget::UNameTagWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Build the widget tree in C++ (no Blueprint asset).
	WidgetTree = NewObject<UWidgetTree>(this, UWidgetTree::StaticClass(), TEXT("WidgetTree"), RF_Transient);

	UVerticalBox* Box = NewObject<UVerticalBox>(WidgetTree, TEXT("RootBox"));

	NameText = NewObject<UTextBlock>(Box, TEXT("NameText"));
	NameText->SetText(FText::FromString(TEXT("?")));
	NameText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	NameText->SetShadowOffset(FVector2D(1.f, 1.f));
	NameText->SetShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.9f));
	FSlateFontInfo NameFont = NameText->GetFont();
	NameFont.Size = 13;
	NameText->SetFont(NameFont);
	Box->AddChild(NameText);

	HealthBar = NewObject<UProgressBar>(Box, TEXT("HealthBar"));
	HealthBar->SetPercent(1.f);
	// Solid white brush so the tints actually render (default style brushes can be empty in game).
	static ConstructorHelpers::FObjectFinder<UTexture2D> WhiteTexAsset(TEXT("/Engine/EngineResources/WhiteSquareTexture"));
	FSlateBrush WhiteBrush;
	WhiteBrush.SetResourceObject(WhiteTexAsset.Succeeded() ? WhiteTexAsset.Object : nullptr);
	WhiteBrush.TintColor = FLinearColor::White;
	HealthBar->WidgetStyle.FillImage = WhiteBrush;
	HealthBar->WidgetStyle.BackgroundImage = WhiteBrush;
	// red background = the HP already lost, green fill = current HP
	HealthBar->WidgetStyle.BackgroundImage.TintColor = FLinearColor(0.75f, 0.05f, 0.05f, 0.9f);
	HealthBar->WidgetStyle.FillImage.TintColor = FLinearColor(0.1f, 0.8f, 0.15f, 1.f);
	HealthBar->SetFillColorAndOpacity(FLinearColor(0.1f, 0.8f, 0.15f, 1.f));
	Box->AddChild(HealthBar);

	WidgetTree->RootWidget = Box;
}

void UNameTagWidget::SetDisplayName(const FString& InName)
{
	if (NameText != nullptr)
	{
		NameText->SetText(FText::FromString(InName));
	}
}

void UNameTagWidget::SetHealthFraction(float Fraction)
{
	if (HealthBar != nullptr)
	{
		HealthBar->SetPercent(FMath::Clamp(Fraction, 0.f, 1.f));
		// fill stays green (current HP); the red background shows the missing part
		const FLinearColor C = FLinearColor(0.1f, 0.8f, 0.15f, 1.f);
		HealthBar->SetFillColorAndOpacity(C);
		HealthBar->WidgetStyle.FillImage.TintColor = C;
	}
}

UWidgetComponent* UNameTagWidget::CreateNameTagComponent(AActor* Owner)
{
	if (Owner == nullptr)
	{
		return nullptr;
	}
	USceneComponent* AttachRoot = Owner->GetRootComponent();
	UWidgetComponent* Comp = NewObject<UWidgetComponent>(Owner, TEXT("NameTag"));
	Comp->SetupAttachment(AttachRoot);
	Comp->SetWidgetSpace(EWidgetSpace::World);
	Comp->SetWidgetClass(UNameTagWidget::StaticClass());
	Comp->SetRelativeLocation(FVector(0.f, 0.f, 175.f));
	Comp->SetDrawSize(FVector2D(130.f, 22.f));
	Comp->SetDrawAtDesiredSize(true);
	Comp->SetPivot(FVector2D(0.5f, 0.f));
	Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Comp->SetVisibility(true);
	Comp->RegisterComponent();
	return Comp;
}

void UNameTagWidget::UpdateNameTag(UWidgetComponent* Comp, const FString& Name, float HealthFraction, const UWorld* World)
{
	if (Comp == nullptr || World == nullptr)
	{
		return;
	}
	// Billboard: face the local player camera.
	if (const APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0))
	{
		if (const APlayerCameraManager* Cam = PC->PlayerCameraManager)
		{
			Comp->SetWorldRotation(FRotator(0.f, Cam->GetCameraRotation().Yaw + 180.f, 0.f));
		}
	}
	if (UNameTagWidget* W = Cast<UNameTagWidget>(Comp->GetWidget()))
	{
		W->SetDisplayName(Name);
		W->SetHealthFraction(HealthFraction);
	}
}
