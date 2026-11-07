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
		MigrateProfile(Loaded); // migration d'ancienne version (vide pour l'instant)
	}

	// USaveGame n'est pas copiable par operator= : on copie champ par champ.
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
	// La cle est le pseudo : volontairement simple et NON securise.
	// TODO(identity): remplacer par un identifiant unique de compte avant toute ouverture au public.
	return UGameplayStatics::SaveGameToSlot(const_cast<UPlayerProfile*>(Profile), Key, 0);
}
