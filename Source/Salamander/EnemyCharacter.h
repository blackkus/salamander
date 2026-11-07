#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "EnemyCharacter.generated.h"

class USalamanderAttributeSet;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<USalamanderAttributeSet> AttributeSet;

private:
	enum class EEnemyState : uint8 { Idle, Chase, Warn, Dead };

	EEnemyState State = EEnemyState::Idle;
	float StateTimer = 0.f;
	float LastAttackTime = 0.f;

	AActor* FindNearestPlayer() const;
	void SetBodyColor(const FLinearColor& Color);
	void DoAttackHit(AActor* Target);
	void OnHealthChanged(const FOnAttributeChangeData& Data);
};
