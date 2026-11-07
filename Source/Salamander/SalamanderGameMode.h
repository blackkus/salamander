#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SalamanderGameMode.generated.h"

UCLASS()
class SALAMANDER_API ASalamanderGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASalamanderGameMode();

	// Refuse la connexion si le pseudo (option ?Name=) est vide, > 16 caracteres ou deja pris.
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

	// Lit ?Name= et ?Color= de l'URL de connexion et les stocke dans le PlayerState (serveur).
	virtual APlayerController* Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

	virtual void PostLogin(APlayerController* NewPlayer) override;

	// Commande console serveur : applique des dégâts à tous les joueurs connectés.
	UFUNCTION(Exec)
	void DebugDamage(int32 Amount);

	// Applique des dégâts à tous les pawns (serveur uniquement).
	void ApplyDamageToAll(int32 Amount);
};
