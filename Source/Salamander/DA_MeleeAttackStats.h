#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DA_MeleeAttackStats.generated.h"

// Melee attack stats. All values PLACEHOLDER until balancing decisions are made.
UCLASS()
class SALAMANDER_API UDA_MeleeAttackStats : public UDataAsset
{
	GENERATED_BODY()

public:
	// PLACEHOLDER: window (s) after a hit during which a re-click continues the combo.
	UPROPERTY(EditAnywhere, Category = "Melee")
	float ComboWindow = 0.6f;

	// PLACEHOLDER: delay (s) between the click and the hit landing (telegraph time).
	UPROPERTY(EditAnywhere, Category = "Melee")
	float HitDelay = 0.15f;

	// PLACEHOLDER: hit sphere radius / reach in cm.
	UPROPERTY(EditAnywhere, Category = "Melee")
	float Range = 160.f;

	// PLACEHOLDER: damage per hit.
	UPROPERTY(EditAnywhere, Category = "Melee")
	float DamagePerHit = 25.f;

	// PLACEHOLDER: cooldown (s) between two swings.
	UPROPERTY(EditAnywhere, Category = "Melee")
	float Cooldown = 0.5f;

	// PLACEHOLDER: number of hits in the combo chain.
	UPROPERTY(EditAnywhere, Category = "Melee")
	int32 MaxCombo = 3;
};
