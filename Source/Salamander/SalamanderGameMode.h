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

	// Login screen (Menu map, local client only).
	void ShowLoginMenu();
	void HideLoginMenu();
	void ConnectToServer(const FString& Ip, const FString& Pseudo, const FString& Color);

	// Server console commands.
	UFUNCTION(Exec)
	void DebugDamage(int32 Amount);

	UFUNCTION(Exec)
	void DebugSetLevel(const FString& Name, int32 NewLevel);

	void ApplyDamageToAll(int32 Amount);

	// Server-side save (one file per player).
	void SaveProfileFor(const class ASalamanderPlayerState* PS);
	void SaveAllProfiles();

	// Player death (HP = 0): destroys the pawn and respawns at the start point after 3 s.
	void HandlePlayerDeath(class APawn* Pawn);

	// Boss killed: gives the reward (DT_BossReward) to every connected player and saves profiles.
	void OnBossKilled(class ABossCharacter* Boss);

private:
	TSharedPtr<class SLoginWidget> LoginWidget;

	UPROPERTY()
	TObjectPtr<UObject> SaveBackendObj;

	class ISaveBackend* SaveBackend = nullptr;

	// Death rules (PROVISOIRE values inside).
	UPROPERTY()
	TObjectPtr<class UDA_DeathRules> DeathRules;

	FTimerHandle SaveTimerHandle;
};
