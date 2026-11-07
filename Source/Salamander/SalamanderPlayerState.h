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

	// Pseudo choisi a la connexion (option d'URL ?Name=...). Serveur uniquement.
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Identity")
	FString PlayerName;

	// Couleur choisie a la connexion (option d'URL ?Color=RRGGBB), appliquee au mesh.
	UPROPERTY(ReplicatedUsing=OnRep_Color, BlueprintReadOnly, Category="Identity")
	FLinearColor ColorRgb;

	UFUNCTION()
	void OnRep_Color();

	// Cree un MID depuis M_PlayerColor (parametre BodyColor) et l'applique au mesh du pawn.
	void ApplyColorToPawn();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
};
