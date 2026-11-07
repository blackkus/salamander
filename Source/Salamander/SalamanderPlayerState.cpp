#include "SalamanderPlayerState.h"

#include "Net/UnrealNetwork.h"

ASalamanderPlayerState::ASalamanderPlayerState()
{
	Level = 1;
}

void ASalamanderPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && Level < 1)
	{
		Level = 1;
	}
}

void ASalamanderPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASalamanderPlayerState, Level);
}
