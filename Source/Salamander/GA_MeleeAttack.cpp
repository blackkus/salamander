#include "GA_MeleeAttack.h"

#include "AbilitySystemComponent.h"
#include "DrawDebugHelpers.h"
#include "GameplayTagContainer.h"
#include "TimerManager.h"
#include "DA_MeleeAttackStats.h"
#include "GE_Damage.h"
#include "SalamanderCharacter.h"

UGA_MeleeAttack::UGA_MeleeAttack()
{
	// Tag used to trigger this ability from the input action.
	// no error if not registered yet: native tags are registered in module StartupModule, after CDO construction
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.MeleeAttack"), false));
	// Instant-like ability: we end it right away; the hit lands after HitDelay via a timer.
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

bool UGA_MeleeAttack::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	// Server refuses the attack if the player is dead or the cooldown is not elapsed.
	if (const UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Dead"))))
		{
			return false;
		}
	}
	bool bDead = false;
	if (const UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		bDead = ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Dead")));
	}
	bool bCooldown = false;
	if (const ASalamanderCharacter* Char = Cast<ASalamanderCharacter>(ActorInfo->AvatarActor.Get()))
	{
		if (const UDA_MeleeAttackStats* Stats = LoadObject<UDA_MeleeAttackStats>(nullptr, TEXT("/Game/Data/DA_MeleeAttackStats")))
		{
			bCooldown = Char->GetWorld() && Char->GetWorld()->GetTimeSeconds() - Char->LastMeleeTime < Stats->Cooldown;
		}
	}
	const bool bSuper = Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
	UE_LOG(LogTemp, Log, TEXT("[Melee] CanActivate dead=%d cooldown=%d super=%d"), bDead ? 1 : 0, bCooldown ? 1 : 0, bSuper ? 1 : 0);
	if (bDead || bCooldown)
	{
		return false;
	}
	return bSuper;
}

void UGA_MeleeAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	AActor* Avatar = ActorInfo->AvatarActor.Get();
	if (ASC == nullptr || Avatar == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const UDA_MeleeAttackStats* Stats = LoadObject<UDA_MeleeAttackStats>(nullptr, TEXT("/Game/Data/DA_MeleeAttackStats"));
	const float ComboWindow = Stats ? Stats->ComboWindow : 0.6f;
	const float HitDelay = Stats ? Stats->HitDelay : 0.15f;
	const float Range = Stats ? Stats->Range : 160.f;
	const float Damage = Stats ? Stats->DamagePerHit : 25.f;
	const int32 MaxCombo = Stats ? Stats->MaxCombo : 3;

	UWorld* World = Avatar->GetWorld();
	ASalamanderCharacter* Char = Cast<ASalamanderCharacter>(Avatar);

	// Combo chain (server authoritative).
	if (Char != nullptr && World != nullptr)
	{
		const float Now = World->GetTimeSeconds();
		if (Now - Char->LastMeleeTime <= ComboWindow)
		{
			Char->MeleeComboIndex = (Char->MeleeComboIndex + 1) % FMath::Max(1, MaxCombo);
		}
		else
		{
			Char->MeleeComboIndex = 0;
		}
		Char->LastMeleeTime = Now;
		UE_LOG(LogTemp, Log, TEXT("[Melee] swing combo=%d (server=%d)"), Char->MeleeComboIndex, Avatar->HasAuthority() ? 1 : 0);
	}

	// Debug visual: a line in front of the player (no animation yet).
	const FVector Loc = Avatar->GetActorLocation();
	const FVector Fwd = Avatar->GetActorForwardVector();
	DrawDebugLine(World, Loc, Loc + Fwd * Range, FColor::Red, false, HitDelay + 0.2f, 0, 4.f);

	// Server: schedule the hit after the telegraph delay.
	if (Avatar->HasAuthority() && World != nullptr)
	{
		TWeakObjectPtr<UAbilitySystemComponent> WeakASC = ASC;
		TWeakObjectPtr<AActor> WeakAvatar = Avatar;
		FTimerHandle TimerHandle;
		World->GetTimerManager().SetTimer(TimerHandle, [this, WeakASC, WeakAvatar, Range, Damage]()
		{
			if (UAbilitySystemComponent* ASC2 = WeakASC.Get(); ASC2 != nullptr && WeakAvatar.IsValid())
			{
				DoHitServer(ASC2, WeakAvatar.Get(), Range, Damage);
			}
		}, HitDelay, false);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_MeleeAttack::DoHitServer(UAbilitySystemComponent* ASC, AActor* Avatar, float Range, float Damage)
{
	UWorld* World = Avatar->GetWorld();
	if (World == nullptr)
	{
		return;
	}

	// Generous sphere sweep in front of the player, along the view direction.
	const FVector Loc = Avatar->GetActorLocation();
	const FVector Fwd = Avatar->GetActorForwardVector();
	const FVector Center = Loc + Fwd * (Range * 0.5f);
	const FCollisionShape Shape = FCollisionShape::MakeSphere(Range * 0.5f);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(MeleeHit), false, Avatar);
	Params.AddIgnoredActor(Avatar);

	TArray<FHitResult> Hits;
	World->SweepMultiByChannel(Hits, Loc, Center, FQuat::Identity, ECC_Pawn, Shape, Params);

	// One swing hits a given target only once.
	TSet<AActor*> HitActors;
	for (const FHitResult& Hit : Hits)
	{
		AActor* Other = Hit.GetActor();
		if (Other == nullptr || Other == Avatar || HitActors.Contains(Other))
		{
			continue;
		}
		HitActors.Add(Other);
		if (UAbilitySystemComponent* TargetASC = Other->FindComponentByClass<UAbilitySystemComponent>())
		{
			UGE_Damage::ApplyTo(TargetASC, Damage);
			UE_LOG(LogTemp, Log, TEXT("[Melee] hit %s for %.0f"), *Other->GetName(), Damage);
		}
	}
	if (HitActors.Num() == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("[Melee] swing hit nothing"));
	}
}
