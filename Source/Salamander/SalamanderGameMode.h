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

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual APlayerController* Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	// Commandes console serveur.
	UFUNCTION(Exec)
	void DebugDamage(int32 Amount);

	UFUNCTION(Exec)
	void DebugSetLevel(const FString& Name, int32 NewLevel);

	void ApplyDamageToAll(int32 Amount);

	// Sauvegarde serveur (un fichier par joueur).
	void SaveProfileFor(const class ASalamanderPlayerState* PS);
	void SaveAllProfiles();

private:
	UPROPERTY()
	TObjectPtr<UObject> SaveBackendObj;

	class ISaveBackend* SaveBackend = nullptr;

	FTimerHandle SaveTimerHandle;
};
