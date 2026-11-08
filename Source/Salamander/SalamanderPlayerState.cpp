#include "SalamanderPlayerState.h"

#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "SalamanderAttributeSet.h"

ASalamanderPlayerState::ASalamanderPlayerState()
{
	Level = 1;
	XP = 0;
	ColorRgb = FLinearColor(0.8f, 0.8f, 0.8f, 1.f);

	// GAS: ASC + AttributeSet on the PlayerState, Mixed replication (players).
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<USalamanderAttributeSet>(TEXT("AttributeSet"));
}

void ASalamanderPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && Level < 1)
	{
		Level = 1;
	XP = 0;
	}
}

void ASalamanderPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASalamanderPlayerState, Level);
	DOREPLIFETIME(ASalamanderPlayerState, XP);
	DOREPLIFETIME(ASalamanderPlayerState, PlayerName);
	DOREPLIFETIME_CONDITION_NOTIFY(ASalamanderPlayerState, ColorRgb, COND_None, REPNOTIFY_Always);
}

void ASalamanderPlayerState::OnRep_Color()
{
	UE_LOG(LogTemp, Log, TEXT("[PS] OnRep_Color %s -> %s"), *PlayerName, *ColorRgb.ToString());
	ApplyColorToPawn();
}

void ASalamanderPlayerState::ApplyColorToPawn()
{
	APawn* Pawn = GetPawn();
	if (Pawn == nullptr)
	{
		return;
	}
	USkeletalMeshComponent* Mesh = Pawn->FindComponentByClass<USkeletalMeshComponent>();
	if (Mesh == nullptr)
	{
		return;
	}

	UMaterialInterface* BaseMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_PlayerColor"));
	if (BaseMat == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PS] M_PlayerColor not found"));
		return;
	}

	UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMat, this);
	MID->SetVectorParameterValue(TEXT("BodyColor"), ColorRgb);
	Mesh->SetMaterial(0, MID);
}
