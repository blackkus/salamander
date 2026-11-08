#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "EnemyCharacter.generated.h"

class USalamanderAttributeSet;
class UWidgetComponent;
class UDamageType;

// Simple enemy: C++ state machine (Idle / Chase / Warn / Attack), own ASC (Minimal replication).
// No Behavior Tree. Telegraphs its attack by flashing its body color before hitting.
UCLASS()
class SALAMANDER_API AEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void FellOutOfWorld(const UDamageType& DamageType) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<USalamanderAttributeSet> AttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UWidgetComponent> NameTagComponent;

protected:
	enum class EEnemyState : uint8 { Idle, Chase, Warn, Dead };

	EEnemyState State = EEnemyState::Idle;
	float StateTimer = 0.f;
	float LastAttackTime = 0.f;

	AActor* FindNearestPlayer() const;
	void SetBodyColor(const FLinearColor& Color);
	void DoAttackHit(AActor* Target);
	void OnHealthChanged(const FOnAttributeChangeData& Data);

public:
	// Overhead name tag label ("Enemy", boss overrides to "Boss").
	virtual FString GetDisplayName() const { return TEXT("Enemy"); }
	// Health fraction [0,1] from the own ASC.
	float GetHealthFraction() const;
};
