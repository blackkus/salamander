#include "SalamanderAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"
#include "GameplayTagContainer.h"
#include "SalamanderGameMode.h"

USalamanderAttributeSet::USalamanderAttributeSet()
{
	// Defaults: a fresh attribute set must not read 0 HP (would trigger the death path before GE_InitStats).
	Health.SetBaseValue(100.f);
	Health.SetCurrentValue(100.f);
	MaxHealth.SetBaseValue(100.f);
	MaxHealth.SetCurrentValue(100.f);
}

void USalamanderAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(USalamanderAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USalamanderAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

void USalamanderAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USalamanderAttributeSet, Health, OldHealth);
	UE_LOG(LogTemp, Log, TEXT("[Attr] OnRep Health %.0f -> %.0f"), OldHealth.GetCurrentValue(), Health.GetCurrentValue());
}

void USalamanderAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USalamanderAttributeSet, MaxHealth, OldMaxHealth);
}

void USalamanderAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		// Clamp Health into [0, MaxHealth] (server side).
		Health.SetCurrentValue(FMath::Clamp(Health.GetCurrentValue(), 0.f, MaxHealth.GetCurrentValue()));
	}

	if (Data.EvaluatedData.Attribute == GetHealthAttribute() && Health.GetCurrentValue() <= 0.f)
	{
		// Death: notify the GameMode, server side only.
		if (UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent())
		{
			if (APawn* Pawn = Cast<APawn>(ASC->GetAvatarActor()); Pawn != nullptr && Pawn->HasAuthority())
			{
				ASC->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Dead")));
				if (ASalamanderGameMode* GM = Pawn->GetWorld()->GetAuthGameMode<ASalamanderGameMode>())
				{
					GM->HandlePlayerDeath(Pawn);
				}
			}
		}
	}
}
