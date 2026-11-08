#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DA_BossStats.generated.h"

// Boss stats. All values PLACEHOLDER until balancing decisions are made.
UCLASS()
class SALAMANDER_API UDA_BossStats : public UDataAsset
{
	GENERATED_BODY()

public:
	// PLACEHOLDER: boss health.
	UPROPERTY(EditAnywhere, Category = "Boss")
	float Health = 300.f;

	// PLACEHOLDER: move speed (cm/s).
	UPROPERTY(EditAnywhere, Category = "Boss")
	float MoveSpeed = 220.f;

	// PLACEHOLDER: distance (cm) at which the boss stops and attacks.
	UPROPERTY(EditAnywhere, Category = "Boss")
	float AttackRange = 200.f;

	// PLACEHOLDER: light attack damage.
	UPROPERTY(EditAnywhere, Category = "Boss")
	float AttackDamage = 20.f;

	// PLACEHOLDER: light attack telegraph (s).
	UPROPERTY(EditAnywhere, Category = "Boss")
	float WarnDuration = 0.8f;

	// PLACEHOLDER: cooldown (s) between attacks.
	UPROPERTY(EditAnywhere, Category = "Boss")
	float AttackCooldown = 1.2f;

	// PLACEHOLDER: slam (AoE) damage.
	UPROPERTY(EditAnywhere, Category = "Boss|Slam")
	float SlamDamage = 35.f;

	// PLACEHOLDER: slam radius (cm).
	UPROPERTY(EditAnywhere, Category = "Boss|Slam")
	float SlamRadius = 350.f;

	// PLACEHOLDER: slam telegraph (s), longer than the light attack.
	UPROPERTY(EditAnywhere, Category = "Boss|Slam")
	float SlamWarnDuration = 1.5f;
};
