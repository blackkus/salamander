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

	virtual void PostLogin(APlayerController* NewPlayer) override;

	// Commande console serveur : applique des dégâts à tous les joueurs connectés.
	UFUNCTION(Exec)
	void DebugDamage(int32 Amount);
};
