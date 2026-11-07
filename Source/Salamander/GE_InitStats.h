#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GE_InitStats.generated.h"

// Sets Health and MaxHealth to 100 (Instant, Override). Applied server-side on possession.
UCLASS()
class SALAMANDER_API UGE_InitStats : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGE_InitStats();
};
