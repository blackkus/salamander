#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SaveBackend.generated.h"

UINTERFACE(BlueprintType)
class USaveBackend : public UInterface
{
	GENERATED_BODY()
};

// Interface de sauvegarde des profils (un fichier par joueur sur le disque du serveur).
class SALAMANDER_API ISaveBackend
{
	GENERATED_BODY()

public:
	virtual bool LoadProfile(const FString& Key, class UPlayerProfile* OutProfile) = 0;
	virtual bool SaveProfile(const FString& Key, const class UPlayerProfile* Profile) = 0;

	// Migration d'une sauvegarde d'ancienne version (vide pour l'instant).
	virtual void MigrateProfile(class UPlayerProfile* Profile) {}
};
