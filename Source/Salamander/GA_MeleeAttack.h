#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_MeleeAttack.generated.h"

// Melee attack ability (3-hit combo). Client activates locally (prediction), server validates
// and does the hit detection (sphere sweep in front of the player) after a telegraph delay.
UCLASS()
class SALAMANDER_API UGA_MeleeAttack : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_MeleeAttack();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	void DoHitServer(class UAbilitySystemComponent* ASC, class AActor* Avatar, float Range, float Damage);
};
