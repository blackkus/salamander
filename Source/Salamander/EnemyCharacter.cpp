#include "EnemyCharacter.h"

#include "AbilitySystemComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/PlayerState.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DA_EnemyStats.h"
#include "GE_Damage.h"
#include "SalamanderAttributeSet.h"
#include "BossCharacter.h"
#include "SalamanderGameMode.h"
#include "Components/WidgetComponent.h"
#include "NameTagWidget.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/Controller.h"

AEnemyCharacter::AEnemyCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bUseControllerRotationYaw = true; // the enemy turns to face its target (set from Tick)
	// Own ASC on the character (Minimal replication for enemies).
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AttributeSet = CreateDefaultSubobject<USalamanderAttributeSet>(TEXT("AttributeSet"));

	// Provisional body: same mannequin as the player, tinted red via M_PlayerColor (BodyColor).
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -90.f), FRotator(0.f, -90.f, 0.f));
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple"));
	if (MeshAsset.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshAsset.Object);
	}
	static ConstructorHelpers::FObjectFinder<UAnimBlueprint> AnimAsset(TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed"));
	if (AnimAsset.Succeeded())
	{
		GetMesh()->SetAnimInstanceClass(AnimAsset.Object->GeneratedClass);
	}

	bUseControllerRotationYaw = false;
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Overhead name tag + health bar (created on every client).
	NameTagComponent = UNameTagWidget::CreateNameTagComponent(this);

	if (HasAuthority())
	{
		if (AbilitySystemComponent != nullptr)
		{
			AbilitySystemComponent->InitAbilityActorInfo(this, this);

			// Init attributes from DA_EnemyStats via a runtime GE (values are PLACEHOLDER in the DA).
			const UDA_EnemyStats* Stats = LoadObject<UDA_EnemyStats>(nullptr, TEXT("/Game/Data/DA_EnemyStats"));
			const float EnemyHealth = Stats ? Stats->Health : 60.f;
			if (UGameplayEffect* GE = NewObject<UGameplayEffect>(GetTransientPackage()))
			{
				GE->DurationPolicy = EGameplayEffectDurationType::Instant;
				FGameplayModifierInfo HealthMod;
				HealthMod.Attribute = USalamanderAttributeSet::GetHealthAttribute();
				HealthMod.ModifierOp = EGameplayModOp::Override;
				HealthMod.ModifierMagnitude = FScalableFloat(EnemyHealth);
				GE->Modifiers.Add(HealthMod);
				FGameplayModifierInfo MaxMod;
				MaxMod.Attribute = USalamanderAttributeSet::GetMaxHealthAttribute();
				MaxMod.ModifierOp = EGameplayModOp::Override;
				MaxMod.ModifierMagnitude = FScalableFloat(EnemyHealth);
				GE->Modifiers.Add(MaxMod);

				FGameplayEffectContextHandle Ctx = AbilitySystemComponent->MakeEffectContext();
				Ctx.AddSourceObject(this);
				FGameplayEffectSpec Spec(GE, Ctx, 1.f);
				AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(Spec);
			}

			// Death on Health <= 0.
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(USalamanderAttributeSet::GetHealthAttribute()).AddUObject(this, &AEnemyCharacter::OnHealthChanged);

			if (Stats)
			{
				GetCharacterMovement()->MaxWalkSpeed = Stats->MoveSpeed;
			}
		}
		SetBodyColor(FLinearColor(0.7f, 0.1f, 0.1f, 1.f));
		UE_LOG(LogTemp, Log, TEXT("[Enemy] spawned %s"), *GetName());
	}
}

void AEnemyCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Name tag on every machine.
	if (NameTagComponent != nullptr)
	{
		UNameTagWidget::UpdateNameTag(NameTagComponent, GetDisplayName(), GetHealthFraction(), GetWorld());
	}

	if (!HasAuthority())
	{
		return;
	}

	const UDA_EnemyStats* Stats = LoadObject<UDA_EnemyStats>(nullptr, TEXT("/Game/Data/DA_EnemyStats"));
	const float AttackRange = Stats ? Stats->AttackRange : 140.f;
	const float WarnDuration = Stats ? Stats->WarnDuration : 0.8f;
	const float AttackCooldown = Stats ? Stats->AttackCooldown : 1.5f;
	const float AggroRange = 1500.f; // PLACEHOLDER

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
			State = EEnemyState::Warn;
			StateTimer = WarnDuration;
			SetBodyColor(FLinearColor(1.f, 0.f, 0.f, 1.f)); // telegraph: flash red
			UE_LOG(LogTemp, Log, TEXT("[Enemy] %s warns (telegraph %.1fs)"), *GetName(), WarnDuration);
			break;
		}
		// Chase directly (no navmesh yet) toward the nearest player.
		const FVector Dir = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		// Face the target: never walk backwards toward the player.
		const FRotator FaceRot = FRotationMatrix::MakeFromX(Dir).Rotator();
		const FRotator NewRot = FMath::RInterpTo(GetActorRotation(), FaceRot, DeltaSeconds, 12.f);
		if (AController* C = GetController())
		{
			C->SetControlRotation(NewRot); // movement input is transformed by control rotation
		}
		SetActorRotation(NewRot);
		AddMovementInput(Dir, 1.f);
		break;
	}

	case EEnemyState::Warn:
		// Keep facing the target during the telegraph.
		if (Target != nullptr)
		{
			const FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
			const FRotator FaceRot = FRotationMatrix::MakeFromX(ToTarget.GetSafeNormal()).Rotator();
			const FRotator NewRot = FMath::RInterpTo(GetActorRotation(), FaceRot, DeltaSeconds, 12.f);
			if (AController* C = GetController())
			{
				C->SetControlRotation(NewRot);
			}
			SetActorRotation(NewRot);
		}
		StateTimer -= DeltaSeconds;
		if (StateTimer <= 0.f)
		{
			State = EEnemyState::Chase;
			SetBodyColor(FLinearColor(0.7f, 0.1f, 0.1f, 1.f));
			if (Target != nullptr && FVector::Dist(GetActorLocation(), Target->GetActorLocation()) < AttackRange * 1.5f
				&& GetWorld()->GetTimeSeconds() - LastAttackTime >= AttackCooldown)
			{
				LastAttackTime = GetWorld()->GetTimeSeconds();
				DoAttackHit(Target);
			}
		}
		break;

	case EEnemyState::Dead:
		break;
	}
}

float AEnemyCharacter::GetHealthFraction() const
{
	if (AttributeSet != nullptr)
	{
		const float H = AttributeSet->GetHealth();
		const float M = AttributeSet->GetMaxHealth();
		return M > 0.f ? H / M : 1.f;
	}
	return 1.f;
}

AActor* AEnemyCharacter::FindNearestPlayer() const
{
	AActor* Best = nullptr;
	float BestDist = FLT_MAX;
	if (const UWorld* World = GetWorld())
	{
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			if (const APawn* P = It->Get()->GetPawn())
			{
				const float D = FVector::Dist(GetActorLocation(), P->GetActorLocation());
				if (D < BestDist)
				{
					BestDist = D;
					Best = const_cast<APawn*>(P);
				}
			}
		}
	}
	return Best;
}

void AEnemyCharacter::SetBodyColor(const FLinearColor& Color)
{
	UMaterialInterface* BaseMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_PlayerColor"));
	if (BaseMat == nullptr)
	{
		return;
	}
	UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMat, this);
	MID->SetVectorParameterValue(TEXT("BodyColor"), Color);
	GetMesh()->SetMaterial(0, MID);
}

void AEnemyCharacter::DoAttackHit(AActor* Target)
{
	const UDA_EnemyStats* Stats = LoadObject<UDA_EnemyStats>(nullptr, TEXT("/Game/Data/DA_EnemyStats"));
	const float Damage = Stats ? Stats->AttackDamage : 15.f;

	if (UAbilitySystemComponent* TargetASC = Target->FindComponentByClass<UAbilitySystemComponent>())
	{
		UGE_Damage::ApplyTo(TargetASC, Damage);
		UE_LOG(LogTemp, Log, TEXT("[Enemy] %s hits %s for %.0f"), *GetName(), *Target->GetName(), Damage);
	}
}

void AEnemyCharacter::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue <= 0.f && State != EEnemyState::Dead)
	{
		State = EEnemyState::Dead;
		UE_LOG(LogTemp, Log, TEXT("[Enemy] %s died"), *GetName());
		// A boss dying gives the reward to all connected players (server).
		if (ABossCharacter* Boss = Cast<ABossCharacter>(this))
		{
			if (ASalamanderGameMode* GM = GetWorld()->GetAuthGameMode<ASalamanderGameMode>())
			{
				GM->OnBossKilled(Boss);
			}
		}
		Destroy(); // at 0 HP the enemy disappears
	}
}

void AEnemyCharacter::FellOutOfWorld(const UDamageType& DamageType)
{
	UE_LOG(LogTemp, Warning, TEXT("[Enemy] %s fell out of the world at %s -> destroyed"), *GetName(), *GetActorLocation().ToString());
	Super::FellOutOfWorld(DamageType);
}
