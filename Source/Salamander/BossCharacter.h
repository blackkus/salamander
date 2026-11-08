#pragma once

#include "CoreMinimal.h"
#include "EnemyCharacter.h"
#include "BossCharacter.generated.h"

// Boss: same state machine as the simple enemy, but alternates a light attack and an AoE slam
// (each with its own telegraph). All values in DA_BossStats (PLACEHOLDER). Gives a reward on death.
UCLASS()
class SALAMANDER_API ABossCharacter : public AEnemyCharacter
{
	GENERATED_BODY()

public:
	ABossCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	virtual FString GetDisplayName() const override { return TEXT("Boss"); }

private:
	int32 AttackCount = 0; // alternates light attack / slam
	void DoSlam();
};
