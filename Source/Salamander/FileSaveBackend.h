#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SaveBackend.h"
#include "FileSaveBackend.generated.h"

// Sauvegarde dans des fichiers USaveGame, un par joueur (slot = cle), sur le disque du serveur.
UCLASS()
class SALAMANDER_API UFileSaveBackend : public UObject, public ISaveBackend
{
	GENERATED_BODY()

public:
	virtual bool LoadProfile(const FString& Key, UPlayerProfile* OutProfile) override;
	virtual bool SaveProfile(const FString& Key, const UPlayerProfile* Profile) override;
};
