#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NameTagWidget.generated.h"

class UTextBlock;
class UProgressBar;
class UWidgetComponent;
class AActor;

// Overhead name tag: nick (or enemy label) + health bar (no numeric value).
// Widget tree is built in C++ (no Blueprint).
UCLASS()
class SALAMANDER_API UNameTagWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UNameTagWidget(const FObjectInitializer& ObjectInitializer);

	void SetDisplayName(const FString& InName);
	void SetHealthFraction(float Fraction);

	// Creates a world-space widget component attached to the actor mesh, returns it.
	static UWidgetComponent* CreateNameTagComponent(AActor* Owner);
	// Updates name/health and faces the local camera. Call every tick on each client.
	static void UpdateNameTag(UWidgetComponent* Comp, const FString& Name, float HealthFraction, const UWorld* World);

private:
	UPROPERTY()
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY()
	TObjectPtr<UProgressBar> HealthBar;
};
