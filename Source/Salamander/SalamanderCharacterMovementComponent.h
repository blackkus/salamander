#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SalamanderCharacterMovementComponent.generated.h"

// Custom move: carries the "sprint" flag in the compressed bits,
// like bWantsToCrouch, so the sprint speed is predicted on the client.
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
	// Max speed while sprinting (base walk/run = MaxWalkSpeed, unchanged).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sprint")
	float SprintMaxWalkSpeed = 1000.f;

	// Predicted: packed into FSavedMove_Salamander and replayed on client and server.
	bool bSprinting = false;

	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	virtual float GetMaxSpeed() const override;

	// Logs the magnitude of each server -> client correction (prediction test).
	virtual void OnClientCorrectionReceived(FNetworkPredictionData_Client_Character& ClientData, float TimeStamp, FVector NewLocation, FVector NewVelocity, UPrimitiveComponent* NewBase, FName NewBaseBoneName, bool bHasBase, bool bBaseRelativePosition, uint8 ServerMovementMode, FVector ServerGravityDirection) override;
};
