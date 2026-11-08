#include "NameTagWidget.h"

#include "Blueprint/WidgetTree.h"
#include "UObject/UObjectGlobals.h"

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
	NameFont.Size = 18;
	NameText->SetFont(NameFont);
	Box->AddChild(NameText);

	HealthBar = NewObject<UProgressBar>(Box, TEXT("HealthBar"));
	HealthBar->SetPercent(1.f);
	HealthBar->WidgetStyle.BackgroundImage.TintColor = FLinearColor(0.f, 0.f, 0.f, 0.55f);
	HealthBar->WidgetStyle.FillImage.TintColor = FLinearColor(0.15f, 0.85f, 0.2f, 1.f);
	HealthBar->SetFillColorAndOpacity(FLinearColor(0.15f, 0.85f, 0.2f, 1.f));
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
		// red when low
		const FLinearColor C = Fraction > 0.5f ? FLinearColor(0.15f, 0.85f, 0.2f, 1.f)
			: (Fraction > 0.25f ? FLinearColor(0.95f, 0.75f, 0.1f, 1.f) : FLinearColor(0.9f, 0.1f, 0.1f, 1.f));
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
	Comp->SetRelativeLocation(FVector(0.f, 0.f, 110.f));
	Comp->SetDrawSize(FVector2D(180.f, 36.f));
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
