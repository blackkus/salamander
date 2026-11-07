#include "FileSaveBackend.h"

#include "Kismet/GameplayStatics.h"
#include "PlayerProfile.h"

bool UFileSaveBackend::LoadProfile(const FString& Key, UPlayerProfile* OutProfile)
{
	if (OutProfile == nullptr || !UGameplayStatics::DoesSaveGameExist(Key, 0))
	{
		return false;
	}

	UPlayerProfile* Loaded = Cast<UPlayerProfile>(UGameplayStatics::LoadGameFromSlot(Key, 0));
	if (Loaded == nullptr)
	{
		return false;
	}

	if (Loaded->SaveVersion < 1)
	{
		MigrateProfile(Loaded); // old-version migration (empty for now)
	}

	// USaveGame is not copyable via operator=: copy field by field.
	OutProfile->SaveVersion = Loaded->SaveVersion;
	OutProfile->Name = Loaded->Name;
	OutProfile->ColorRgb = Loaded->ColorRgb;
	OutProfile->Level = Loaded->Level;
	return true;
}

bool UFileSaveBackend::SaveProfile(const FString& Key, const UPlayerProfile* Profile)
{
	if (Profile == nullptr || Key.IsEmpty())
	{
		return false;
	}
	// The key is the pseudo: deliberately simple and NOT secure.
	// TODO(identity): replace with a unique account id before any public release.
	return UGameplayStatics::SaveGameToSlot(const_cast<UPlayerProfile*>(Profile), Key, 0);
}
