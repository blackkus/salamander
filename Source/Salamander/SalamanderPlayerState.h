#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemComponent.h"
#include "SalamanderPlayerState.generated.h"

class USalamanderAttributeSet;

UCLASS()
class SALAMANDER_API ASalamanderPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ASalamanderPlayerState();

	// GAS: the ASC lives on the PlayerState (survives death/respawn). Mixed replication.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="GAS")
	TObjectPtr<USalamanderAttributeSet> AttributeSet;

	UAbilitySystemComponent* GetASC() const { return AbilitySystemComponent; }

	// Player level. Server only.
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Stats")
	int32 Level;

	// XP (rewards). Server only.
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Stats")
	int32 XP;

	// Pseudo chosen at connect (URL option ?Pseudo= / ?Name=...). Server only.
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Identity")
	FString PlayerName;

	// Color chosen at connect (?Color=RRGGBB), applied to the mesh.
	UPROPERTY(ReplicatedUsing=OnRep_Color, BlueprintReadOnly, Category="Identity")
	FLinearColor ColorRgb;

	UFUNCTION()
	void OnRep_Color();

	// Creates a MID from M_PlayerColor (BodyColor param) and applies it to the pawn mesh.
	void ApplyColorToPawn();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
};
