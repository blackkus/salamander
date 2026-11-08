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

	if (Loaded->SaveVersion < 2)
	{
		MigrateProfile(Loaded); // v1 -> v2: adds XP/Items defaults
	}

	// USaveGame is not copyable via operator=: copy field by field.
	OutProfile->SaveVersion = Loaded->SaveVersion;
	OutProfile->XP = Loaded->XP;
	OutProfile->Items = Loaded->Items;
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
	// The key is the nick: deliberately simple and NOT secure.
	// TODO(identity): replace with a unique account id before any public release.
	return UGameplayStatics::SaveGameToSlot(const_cast<UPlayerProfile*>(Profile), Key, 0);
}


void UFileSaveBackend::MigrateProfile(UPlayerProfile* Profile)
{
	// v1 -> v2: profile had no XP/Items; defaults are fine, just bump the version.
	if (Profile != nullptr && Profile->SaveVersion < 2)
	{
		const int32 OldVersion = Profile->SaveVersion;
		Profile->SaveVersion = 2;
		UE_LOG(LogTemp, Log, TEXT("[Save] migrated profile %s from v%d to v2"), *Profile->Name, OldVersion);
	}
}
