#include "StatsComponent.h"

#include "Net/UnrealNetwork.h"
#include "SalamanderGameMode.h"

UStatsComponent::UStatsComponent()
{
	MaxHealth = 100;
	Health = 100;
	SetIsReplicatedByDefault(true);
}

void UStatsComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner()->HasAuthority())
	{
		Health = MaxHealth;
	}
}

void UStatsComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UStatsComponent, Health, COND_None, REPNOTIFY_Always);
}

void UStatsComponent::ApplyDamage(int32 Amount)
{
	// ServerOnly : un client qui appellerait ceci n'aurait aucun effet.
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	const int32 OldHealth = Health;
	Health = FMath::Clamp(Health - Amount, 0, MaxHealth);

	// Sur le serveur, OnRep n'est pas déclenché automatiquement : on l'appelle pour le log.
	OnRep_Health(OldHealth);

	// Mort : notifie le GameMode (serveur autoritaire) une seule fois.
	if (Health == 0 && OldHealth > 0)
	{
		if (ASalamanderGameMode* GM = GetWorld()->GetAuthGameMode<ASalamanderGameMode>())
		{
			GM->HandlePlayerDeath(Cast<APawn>(GetOwner()));
		}
	}
}

void UStatsComponent::OnRep_Health(int32 OldHealth)
{
	UE_LOG(LogTemp, Log, TEXT("[Stats] %s Health %d -> %d"), *GetOwner()->GetName(), OldHealth, Health);
}

// (voir ApplyDamage) — la notification de mort est faite dans ApplyDamage.
