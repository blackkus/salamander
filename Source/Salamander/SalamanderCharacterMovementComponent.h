#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SalamanderCharacterMovementComponent.generated.h"

// Move custom : transporte le flag "sprint" dans les bits compressés,
// comme bWantsToCrouch, pour que la vitesse de sprint soit prédite côté client.
class FSavedMove_Salamander : public FSavedMove_Character
{
public:
	typedef FSavedMove_Character Super;

	bool bSavedSprint = false;

	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
	virtual void Clear() override;
	virtual uint8 GetCompressedFlags() const override;
	virtual void PrepMoveFor(ACharacter* C) override;
	virtual void SetMoveFor(ACharacter* C, float InDeltaTime, const FVector& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;
};

class FNetworkPredictionData_Client_Salamander : public FNetworkPredictionData_Client_Character
{
public:
	typedef FNetworkPredictionData_Client_Character Super;

	FNetworkPredictionData_Client_Salamander(const UCharacterMovementComponent& ClientMovement) : Super(ClientMovement) {}

	virtual FSavedMovePtr AllocateNewMove() override;
};

UCLASS()
class SALAMANDER_API USalamanderCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	// Vitesse max en sprint (marche/course de base = MaxWalkSpeed, inchangé).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sprint")
	float SprintMaxWalkSpeed = 1000.f;

	// Prédictible : packé dans FSavedMove_Salamander et rejoué côté client et serveur.
	bool bSprinting = false;

	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	virtual float GetMaxSpeed() const override;

	// Loggue la magnitude de chaque correction serveur -> client (test de prediction).
	virtual void OnClientCorrectionReceived(FNetworkPredictionData_Client_Character& ClientData, float TimeStamp, FVector NewLocation, FVector NewVelocity, UPrimitiveComponent* NewBase, FName NewBaseBoneName, bool bHasBase, bool bBaseRelativePosition, uint8 ServerMovementMode, FVector ServerGravityDirection) override;
};
