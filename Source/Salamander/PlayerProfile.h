#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "PlayerProfile.generated.h"

// Profil sauvegarde d'un joueur, cote serveur uniquement.
UCLASS()
class SALAMANDER_API UPlayerProfile : public USaveGame
{
	GENERATED_BODY()

public:
	// Version du format de sauvegarde (migration via ISaveBackend::MigrateProfile).
	UPROPERTY()
	int32 SaveVersion = 1;

	UPROPERTY()
	FString Name;

	UPROPERTY()
	FLinearColor ColorRgb;

	UPROPERTY()
	int32 Level = 1;
};
