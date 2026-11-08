#include "BossCharacter.h"

#include "DrawDebugHelpers.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "DA_BossStats.h"
#include "GE_Damage.h"
#include "SalamanderAttributeSet.h"
#include "SalamanderGameMode.h"

ABossCharacter::ABossCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ABossCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		const UDA_BossStats* Stats = LoadObject<UDA_BossStats>(nullptr, TEXT("/Game/Data/DA_BossStats"));
		const float BossHealth = Stats ? Stats->Health : 300.f;
		if (AbilitySystemComponent != nullptr)
		{
			if (UGameplayEffect* GE = NewObject<UGameplayEffect>(GetTransientPackage()))
			{
				GE->DurationPolicy = EGameplayEffectDurationType::Instant;
				FGameplayModifierInfo HealthMod;
				HealthMod.Attribute = USalamanderAttributeSet::GetHealthAttribute();
				HealthMod.ModifierOp = EGameplayModOp::Override;
				HealthMod.ModifierMagnitude = FScalableFloat(BossHealth);
				GE->Modifiers.Add(HealthMod);
				FGameplayModifierInfo MaxMod;
				MaxMod.Attribute = USalamanderAttributeSet::GetMaxHealthAttribute();
				MaxMod.ModifierOp = EGameplayModOp::Override;
				MaxMod.ModifierMagnitude = FScalableFloat(BossHealth);
				GE->Modifiers.Add(MaxMod);
				FGameplayEffectContextHandle Ctx = AbilitySystemComponent->MakeEffectContext();
				Ctx.AddSourceObject(this);
				FGameplayEffectSpec Spec(GE, Ctx, 1.f);
				AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(Spec);
			}
			if (Stats)
			{
				GetCharacterMovement()->MaxWalkSpeed = Stats->MoveSpeed;
			}
		}
		SetBodyColor(FLinearColor(0.4f, 0.05f, 0.6f, 1.f)); // purple boss
		UE_LOG(LogTemp, Log, TEXT("[Boss] spawned %s"), *GetName());
	}
}

void ABossCharacter::Tick(float DeltaSeconds)
{
	// Same FSM as the enemy, but attacks alternate light / slam.
	if (!HasAuthority())
	{
		return;
	}

	const UDA_BossStats* Stats = LoadObject<UDA_BossStats>(nullptr, TEXT("/Game/Data/DA_BossStats"));
	const float AttackRange = Stats ? Stats->AttackRange : 200.f;
	const float WarnDuration = Stats ? Stats->WarnDuration : 0.8f;
	const float SlamWarnDuration = Stats ? Stats->SlamWarnDuration : 1.5f;
	const float AttackCooldown = Stats ? Stats->AttackCooldown : 1.2f;
	const float AggroRange = 2500.f; // PROVISOIRE

	AActor* Target = FindNearestPlayer();

	switch (State)
	{
	case EEnemyState::Idle:
		if (Target != nullptr && FVector::Dist(GetActorLocation(), Target->GetActorLocation()) < AggroRange)
		{
			State = EEnemyState::Chase;
		}
		break;

	case EEnemyState::Chase:
	{
		if (Target == nullptr)
		{
			State = EEnemyState::Idle;
			break;
		}
		const float Dist = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
		if (Dist < AttackRange)
		{
			const bool bSlam = (AttackCount % 2) == 1;
			State = EEnemyState::Warn;
			StateTimer = bSlam ? SlamWarnDuration : WarnDuration;
			// Telegraph: bright flash (yellow for slam, red for light).
			SetBodyColor(bSlam ? FLinearColor(1.f, 0.8f, 0.f, 1.f) : FLinearColor(1.f, 0.f, 0.f, 1.f));
			if (bSlam)
			{
				const float SlamRadius = Stats ? Stats->SlamRadius : 350.f;
				DrawDebugSphere(GetWorld(), GetActorLocation(), SlamRadius, 24, FColor::Yellow, false, StateTimer, 0, 3.f);
			}
			UE_LOG(LogTemp, Log, TEXT("[Boss] %s warns %s (%.1fs)"), *GetName(), bSlam ? TEXT("SLAM") : TEXT("light"), StateTimer);
			break;
		}
		const FVector Dir = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		AddMovementInput(Dir, 1.f);
		break;
	}

	case EEnemyState::Warn:
		StateTimer -= DeltaSeconds;
		if (StateTimer <= 0.f)
		{
			State = EEnemyState::Chase;
			SetBodyColor(FLinearColor(0.4f, 0.05f, 0.6f, 1.f));
			if (GetWorld()->GetTimeSeconds() - LastAttackTime >= AttackCooldown)
			{
				LastAttackTime = GetWorld()->GetTimeSeconds();
				if ((AttackCount % 2) == 1)
				{
					DoSlam();
				}
				else if (Target != nullptr)
				{
					DoAttackHit(Target);
				}
				AttackCount++;
			}
		}
		break;

	case EEnemyState::Dead:
		break;
	}
}

void ABossCharacter::DoSlam()
{
	const UDA_BossStats* Stats = LoadObject<UDA_BossStats>(nullptr, TEXT("/Game/Data/DA_BossStats"));
	const float SlamDamage = Stats ? Stats->SlamDamage : 35.f;
	const float SlamRadius = Stats ? Stats->SlamRadius : 350.f;

	// AoE: every player in the radius takes the slam damage.
	TArray<FHitResult> Hits;
	FCollisionShape Shape = FCollisionShape::MakeSphere(SlamRadius);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(BossSlam), false, this);
	GetWorld()->SweepMultiByChannel(Hits, GetActorLocation(), GetActorLocation() + FVector(1.f, 0.f, 0.f), FQuat::Identity, ECC_Pawn, Shape, Params);

	TSet<AActor*> HitActors;
	for (const FHitResult& Hit : Hits)
	{
		AActor* Other = Hit.GetActor();
		if (Other == nullptr || Other == this || HitActors.Contains(Other))
		{
			continue;
		}
		HitActors.Add(Other);
		if (UAbilitySystemComponent* TargetASC = Other->FindComponentByClass<UAbilitySystemComponent>())
		{
			UGE_Damage::ApplyTo(TargetASC, SlamDamage);
			UE_LOG(LogTemp, Log, TEXT("[Boss] %s SLAM hits %s for %.0f"), *GetName(), *Other->GetName(), SlamDamage);
		}
	}
}
