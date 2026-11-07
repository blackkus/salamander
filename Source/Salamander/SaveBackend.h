#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SaveBackend.generated.h"

UINTERFACE(BlueprintType)
class USaveBackend : public UInterface
{
	GENERATED_BODY()
};

// Save backend interface (one file per player on the server disk).
class SALAMANDER_API ISaveBackend
{
	GENERATED_BODY()

public:
	virtual bool LoadProfile(const FString& Key, class UPlayerProfile* OutProfile) = 0;
	virtual bool SaveProfile(const FString& Key, const class UPlayerProfile* Profile) = 0;

	// Migration of an old-version save (empty for now).
	virtual void MigrateProfile(class UPlayerProfile* Profile) {}
};
