#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DA_EnemyStats.generated.h"

// Enemy stats. All values PLACEHOLDER until balancing decisions are made.
UCLASS()
class SALAMANDER_API UDA_EnemyStats : public UDataAsset
{
	GENERATED_BODY()

public:
	// PLACEHOLDER: health of a simple enemy.
	UPROPERTY(EditAnywhere, Category = "Enemy")
	float Health = 60.f;

	// PLACEHOLDER: move speed (cm/s).
	UPROPERTY(EditAnywhere, Category = "Enemy")
	float MoveSpeed = 300.f;

	// PLACEHOLDER: distance (cm) at which the enemy stops and attacks.
	UPROPERTY(EditAnywhere, Category = "Enemy")
	float AttackRange = 140.f;

	// PLACEHOLDER: warning (telegraph) duration (s) before the hit lands.
	UPROPERTY(EditAnywhere, Category = "Enemy")
	float WarnDuration = 0.8f;

	// PLACEHOLDER: damage per hit.
	UPROPERTY(EditAnywhere, Category = "Enemy")
	float AttackDamage = 15.f;

	// PLACEHOLDER: cooldown (s) between two enemy attacks.
	UPROPERTY(EditAnywhere, Category = "Enemy")
	float AttackCooldown = 1.5f;
};
