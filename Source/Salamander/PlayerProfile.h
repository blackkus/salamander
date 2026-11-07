#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "PlayerProfile.generated.h"

// Saved player profile, server side only.
UCLASS()
class SALAMANDER_API UPlayerProfile : public USaveGame
{
	GENERATED_BODY()

public:
	// Save format version (migration via ISaveBackend::MigrateProfile).
	UPROPERTY()
	int32 SaveVersion = 1;

	UPROPERTY()
	FString Name;

	UPROPERTY()
	FLinearColor ColorRgb;

	UPROPERTY()
	int32 Level = 1;
};
