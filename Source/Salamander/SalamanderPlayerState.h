#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SalamanderPlayerState.generated.h"

UCLASS()
class SALAMANDER_API ASalamanderPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ASalamanderPlayerState();

	// Niveau du joueur. Seul le serveur le modifie.
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Stats")
	int32 Level;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
};
