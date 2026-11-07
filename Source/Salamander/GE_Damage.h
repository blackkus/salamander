#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GE_Damage.generated.h"

// Instant damage: Health += SetByCaller(Data.Damage). Applied on the server only.
UCLASS()
class SALAMANDER_API UGE_Damage : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGE_Damage();
};
