#include "SalamanderCharacterMovementComponent.h"

#include "GameFramework/Character.h"

bool FSavedMove_Salamander::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	const FSavedMove_Salamander* Other = static_cast<const FSavedMove_Salamander*>(NewMove.Get());
	if (bSavedSprint != Other->bSavedSprint)
	{
		return false;
	}
	return Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void FSavedMove_Salamander::Clear()
{
	Super::Clear();
	bSavedSprint = false;
}

uint8 FSavedMove_Salamander::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();
	if (bSavedSprint)
	{
		Result |= FLAG_Custom_0;
	}
	return Result;
}

void FSavedMove_Salamander::PrepMoveFor(ACharacter* C)
{
	Super::PrepMoveFor(C);
	if (const USalamanderCharacterMovementComponent* MoveComp = Cast<USalamanderCharacterMovementComponent>(C->GetCharacterMovement()))
	{
		bSavedSprint = MoveComp->bSprinting;
	}
}

void FSavedMove_Salamander::SetMoveFor(ACharacter* C, float InDeltaTime, const FVector& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);
	if (USalamanderCharacterMovementComponent* MoveComp = Cast<USalamanderCharacterMovementComponent>(C->GetCharacterMovement()))
	{
		MoveComp->bSprinting = bSavedSprint;
	}
}

FSavedMovePtr FNetworkPredictionData_Client_Salamander::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_Salamander());
}

FNetworkPredictionData_Client* USalamanderCharacterMovementComponent::GetPredictionData_Client() const
{
	if (ClientPredictionData == nullptr)
	{
		USalamanderCharacterMovementComponent* MutableThis = const_cast<USalamanderCharacterMovementComponent*>(this);
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_Salamander(*this);
	}
	return ClientPredictionData;
}

float USalamanderCharacterMovementComponent::GetMaxSpeed() const
{
	if (bSprinting)
	{
		return SprintMaxWalkSpeed;
	}
	return Super::GetMaxSpeed();
}
