#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "RewardTypes.generated.h"

// One row of the boss reward Data Table (DT_BossReward).
USTRUCT(BlueprintType)
struct SALAMANDER_API FRewardRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 XP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString ItemName;
};
