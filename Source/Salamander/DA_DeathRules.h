#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DA_DeathRules.generated.h"

UENUM(BlueprintType)
enum class ERespawnLocationType : uint8
{
	StartPoint UMETA(DisplayName = "Start point"),
	SavedLocation UMETA(DisplayName = "Saved location (not implemented yet)")
};

// Death rules. All values here are PLACEHOLDER until the open decisions are made (death penalties...).
UCLASS()
class SALAMANDER_API UDA_DeathRules : public UDataAsset
{
	GENERATED_BODY()

public:
	// PLACEHOLDER: respawn delay in seconds.
	UPROPERTY(EditAnywhere, Category = "Death")
	float RespawnDelay = 5.f;

	// PLACEHOLDER: only StartPoint is implemented for now.
	UPROPERTY(EditAnywhere, Category = "Death")
	ERespawnLocationType RespawnLocationType = ERespawnLocationType::StartPoint;
};
